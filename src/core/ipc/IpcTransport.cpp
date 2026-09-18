#include "IpcTransport.h"
#include <spdlog/spdlog.h>
#include <iostream>
#include <sstream>

namespace pdfpro::ipc {

// =============================================================================
// Request/Response/Notification JSON serialization
// =============================================================================

json Request::toJson() const {
    json j = {{"jsonrpc", jsonrpc}, {"method", method}, {"params", params}};
    if (!id.is_null()) j["id"] = id;
    return j;
}

Request Request::fromJson(const json& j) {
    Request r;
    r.jsonrpc = j.value("jsonrpc", "2.0");
    r.method = j.at("method").get<std::string>();
    r.params = j.value("params", json{});
    r.id = j.value("id", json(nullptr));
    return r;
}

json Response::toJson() const {
    json j = {{"jsonrpc", jsonrpc}, {"id", id}};
    if (error.is_null()) {
        j["result"] = result;
    } else {
        j["error"] = error;
    }
    return j;
}

Response Response::fromJson(const json& j) {
    Response r;
    r.jsonrpc = j.value("jsonrpc", "2.0");
    r.id = j.value("id", json(nullptr));
    if (j.contains("result")) r.result = j.at("result");
    if (j.contains("error")) r.error = j.at("error");
    return r;
}

json Notification::toJson() const {
    return {{"jsonrpc", jsonrpc}, {"method", method}, {"params", params}};
}

Notification Notification::fromJson(const json& j) {
    Notification n;
    n.jsonrpc = j.value("jsonrpc", "2.0");
    n.method = j.at("method").get<std::string>();
    n.params = j.value("params", json{});
    return n;
}

// =============================================================================
// StdioTransport
// =============================================================================

StdioTransport::StdioTransport() {
    // Set stdin/stdout to binary mode on Windows
#ifdef _WIN32
    _setmode(_fileno(stdin), _O_BINARY);
    _setmode(_fileno(stdout), _O_BINARY);
#endif
}

StdioTransport::~StdioTransport() {
    stop();
}

bool StdioTransport::start() {
    if (running_.exchange(true)) return true;

    readThread_ = std::thread(&StdioTransport::readLoop, this);
    writeThread_ = std::thread(&StdioTransport::writeLoop, this);

    spdlog::info("StdioTransport started");
    return true;
}

void StdioTransport::stop() {
    if (!running_.exchange(false)) return;

    {
        std::lock_guard<std::mutex> lock(writeMutex_);
        writeQueue_ = std::queue<json>(); // clear queue
    }
    writeCv_.notify_all();

    if (readThread_.joinable()) readThread_.join();
    if (writeThread_.joinable()) writeThread_.join();

    spdlog::info("StdioTransport stopped");
}

bool StdioTransport::send(const json& message) {
    if (!running_.load()) return false;

    std::lock_guard<std::mutex> lock(writeMutex_);
    writeQueue_.push(message);
    writeCv_.notify_one();
    return true;
}

void StdioTransport::setMessageHandler(MessageHandler handler) {
    messageHandler_ = std::move(handler);
}

void StdioTransport::setErrorHandler(ErrorHandler handler) {
    errorHandler_ = std::move(handler);
}

void StdioTransport::readLoop() {
    std::string line;
    while (running_.load() && std::getline(std::cin, line)) {
        if (line.empty()) continue;

        try {
            json message = json::parse(line);
            if (messageHandler_) {
                messageHandler_(message);
            }
        } catch (const json::parse_error& e) {
            if (errorHandler_) {
                errorHandler_("JSON parse error: " + std::string(e.what()) + " | Line: " + line);
            }
        } catch (const std::exception& e) {
            if (errorHandler_) {
                errorHandler_("Message handling error: " + std::string(e.what()));
            }
        }
    }

    if (running_.load() && errorHandler_) {
        errorHandler_("Stdin closed unexpectedly");
    }
    running_.store(false);
    writeCv_.notify_all();
}

void StdioTransport::writeLoop() {
    while (running_.load()) {
        json message;
        {
            std::unique_lock<std::mutex> lock(writeMutex_);
            writeCv_.wait(lock, [this] { return !writeQueue_.empty() || !running_.load(); });
            if (!running_.load() && writeQueue_.empty()) break;

            if (writeQueue_.empty()) continue;
            message = std::move(writeQueue_.front());
            writeQueue_.pop();
        }

        try {
            std::string output = message.dump() + "\n";
            std::cout << output << std::flush;
        } catch (const std::exception& e) {
            if (errorHandler_) {
                errorHandler_("Write error: " + std::string(e.what()));
            }
        }
    }
}

// =============================================================================
// JsonRpcClient
// =============================================================================

JsonRpcClient::JsonRpcClient(std::unique_ptr<IpcTransport> transport)
    : transport_(std::move(transport)) {}

JsonRpcClient::~JsonRpcClient() {
    disconnect();
}

bool JsonRpcClient::connect() {
    if (connected_.load()) return true;

    transport_->setMessageHandler([this](const json& msg) { handleMessage(msg); });
    transport_->setErrorHandler([this](const std::string& err) { handleTransportError(err); });

    if (!transport_->start()) {
        lastError_ = "Failed to start transport";
        return false;
    }

    connected_.store(true);
    spdlog::info("JsonRpcClient connected");
    return true;
}

void JsonRpcClient::disconnect() {
    if (!connected_.exchange(false)) return;
    transport_->stop();
    spdlog::info("JsonRpcClient disconnected");
}

std::future<Response> JsonRpcClient::callAsync(const std::string& method, const json& params) {
    json id = nextId_++;
    Request request;
    request.method = method;
    request.params = params;
    request.id = id;

    std::promise<Response> promise;
    auto future = promise.get_future();

    {
        std::lock_guard<std::mutex> lock(pendingMutex_);
        pendingRequests_[id] = std::move(promise);
    }

    transport_->send(request.toJson());
    return future;
}

void JsonRpcClient::notify(const std::string& method, const json& params) {
    Notification notification;
    notification.method = method;
    notification.params = params;
    transport_->send(notification.toJson());
}

void JsonRpcClient::onNotification(const std::string& method, std::function<void(const json&)> handler) {
    notificationHandlers_[method] = std::move(handler);
}

void JsonRpcClient::onProgress(std::function<void(double, const std::string&)> handler) {
    progressHandler_ = std::move(handler);
}

void JsonRpcClient::onError(std::function<void(const std::string&)> handler) {
    errorHandler_ = std::move(handler);
}

void JsonRpcClient::handleMessage(const json& message) {
    try {
        if (message.contains("method")) {
            // Notification
            auto notification = Notification::fromJson(message);
            handleNotification(notification);
        } else if (message.contains("id")) {
            // Response
            auto response = Response::fromJson(message);
            handleResponse(response);
        }
    } catch (const std::exception& e) {
        spdlog::error("Message handling error: {}", e.what());
        if (errorHandler_) errorHandler_(e.what());
    }
}

void JsonRpcClient::handleResponse(const Response& response) {
    std::promise<Response> promise;
    {
        std::lock_guard<std::mutex> lock(pendingMutex_);
        auto it = pendingRequests_.find(response.id);
        if (it != pendingRequests_.end()) {
            promise = std::move(it->second);
            pendingRequests_.erase(it);
        }
    }
    try {
        promise.set_value(response);
    } catch (const std::future_error&) {
        // Promise already satisfied or invalid - ignore
    }
}

void JsonRpcClient::handleNotification(const Notification& notification) {
    if (notification.method == methods::PROGRESS && progressHandler_) {
        double progress = notification.params.value("progress", 0.0);
        std::string message = notification.params.value("message", "");
        progressHandler_(progress, message);
    } else if (notification.method == methods::ERROR && errorHandler_) {
        std::string msg = notification.params.value("message", "Unknown error");
        errorHandler_(msg);
    } else {
        auto it = notificationHandlers_.find(notification.method);
        if (it != notificationHandlers_.end()) {
            it->second(notification.params);
        }
    }
}

void JsonRpcClient::handleTransportError(const std::string& error) {
    lastError_ = error;
    spdlog::error("Transport error: {}", error);
    if (errorHandler_) errorHandler_(error);
}

// =============================================================================
// JsonRpcServer
// =============================================================================

JsonRpcServer::JsonRpcServer(std::unique_ptr<IpcTransport> transport)
    : transport_(std::move(transport)) {}

JsonRpcServer::~JsonRpcServer() {
    stop();
}

bool JsonRpcServer::start() {
    if (running_.exchange(true)) return true;

    transport_->setMessageHandler([this](const json& msg) { handleMessage(msg); });
    transport_->setErrorHandler([this](const std::string& err) { handleTransportError(err); });

    if (!transport_->start()) {
        running_.store(false);
        return false;
    }

    spdlog::info("JsonRpcServer started");
    return true;
}

void JsonRpcServer::stop() {
    if (!running_.exchange(false)) return;
    transport_->stop();
    spdlog::info("JsonRpcServer stopped");
}

void JsonRpcServer::notify(const std::string& method, const json& params) {
    Notification notification;
    notification.method = method;
    notification.params = params;
    transport_->send(notification.toJson());
}

void JsonRpcServer::sendProgress(double progress, const std::string& message) {
    notify(methods::PROGRESS, {{"progress", progress}, {"message", message}});
}

void JsonRpcServer::sendError(const std::string& message, int code) {
    notify(methods::ERROR, {{"message", message}, {"code", code}});
}

void JsonRpcServer::handleMessage(const json& message) {
    try {
        if (message.contains("method") && !message.contains("id")) {
            // Notification
            auto notification = Notification::fromJson(message);
            handleNotification(notification);
        } else if (message.contains("method") && message.contains("id")) {
            // Request
            auto request = Request::fromJson(message);
            handleRequest(request);
        } else if (message.contains("id")) {
            // Response (shouldn't happen on server)
            spdlog::warn("Received response on server side");
        }
    } catch (const std::exception& e) {
        spdlog::error("Server message handling error: {}", e.what());
        handleTransportError(e.what());
    }
}

void JsonRpcServer::handleRequest(const Request& request) {
    Response response;
    response.id = request.id;

    try {
        auto it = methodHandlers_.find(request.method);
        if (it == methodHandlers_.end()) {
            response.error = {{"code", -32601}, {"message", "Method not found: " + request.method}};
        } else {
            response.result = it->second(request.params);
        }
    } catch (const std::exception& e) {
        response.error = {{"code", -32603}, {"message", "Internal error: " + std::string(e.what())}};
    }

    transport_->send(response.toJson());
}

void JsonRpcServer::handleNotification(const Notification& notification) {
    auto it = notificationHandlers_.find(notification.method);
    if (it != notificationHandlers_.end()) {
        it->second(notification.params);
    }
}

void JsonRpcServer::handleTransportError(const std::string& error) {
    spdlog::error("Server transport error: {}", error);
}

Response JsonRpcServer::makeErrorResponse(const json& id, int code, const std::string& message, const json& data) {
    Response response;
    response.id = id;
    response.error = {{"code", code}, {"message", message}, {"data", data}};
    return response;
}

} // namespace pdfpro::ipc