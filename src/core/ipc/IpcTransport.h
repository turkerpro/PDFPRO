#pragma once

#include "IpcProtocol.h"
#include <functional>
#include <future>
#include <thread>
#include <queue>
#include <mutex>
#include <condition_variable>
#include <atomic>
#include <unordered_map>

namespace pdfpro::ipc {

class IpcTransport {
public:
    using MessageHandler = std::function<void(const json&)>;
    using ErrorHandler = std::function<void(const std::string&)>;

    virtual ~IpcTransport() = default;

    virtual bool start() = 0;
    virtual void stop() = 0;
    virtual bool send(const json& message) = 0;
    virtual void setMessageHandler(MessageHandler handler) = 0;
    virtual void setErrorHandler(ErrorHandler handler) = 0;
    virtual bool isRunning() const = 0;
};

class StdioTransport : public IpcTransport {
public:
    StdioTransport();
    ~StdioTransport() override;

    bool start() override;
    void stop() override;
    bool send(const json& message) override;
    void setMessageHandler(MessageHandler handler) override;
    void setErrorHandler(ErrorHandler handler) override;
    bool isRunning() const override { return running_.load(); }

private:
    void readLoop();
    void writeLoop();
    void processLine(const std::string& line);

    std::atomic<bool> running_{false};
    std::thread readThread_;
    std::thread writeThread_;
    std::mutex writeMutex_;
    std::condition_variable writeCv_;
    std::queue<json> writeQueue_;
    MessageHandler messageHandler_;
    ErrorHandler errorHandler_;
    std::atomic<uint64_t> nextRequestId_{1};
};

class JsonRpcClient {
public:
    explicit JsonRpcClient(std::unique_ptr<IpcTransport> transport);
    ~JsonRpcClient();

    bool connect();
    void disconnect();

    // Synchronous call with timeout
    template<typename Params, typename Result>
    std::optional<Result> call(const std::string& method, const Params& params, int timeoutMs = 30000) {
        json paramsJson = params;
        auto future = callAsync(method, paramsJson);
        if (future.wait_for(std::chrono::milliseconds(timeoutMs)) == std::future_status::ready) {
            auto response = future.get();
            if (response.isError()) {
                lastError_ = response.error.dump();
                return std::nullopt;
            }
            try {
                return response.result.get<Result>();
            } catch (...) {
                lastError_ = "Failed to parse response";
                return std::nullopt;
            }
        }
        lastError_ = "Request timeout";
        return std::nullopt;
    }

    // Async call
    std::future<Response> callAsync(const std::string& method, const json& params);

    // Notifications (fire and forget)
    void notify(const std::string& method, const json& params);

    // Event handlers
    void onNotification(const std::string& method, std::function<void(const json&)> handler);
    void onProgress(std::function<void(double, const std::string&)> handler);
    void onError(std::function<void(const std::string&)> handler);

    const std::string& lastError() const { return lastError_; }
    bool isConnected() const { return connected_.load(); }

private:
    void handleResponse(const Response& response);
    void handleNotification(const Notification& notification);
    void handleMessage(const json& message);
    void handleTransportError(const std::string& error);

    std::unique_ptr<IpcTransport> transport_;
    std::atomic<bool> connected_{false};
    std::mutex pendingMutex_;
    
    // Custom hash for nlohmann::json
    struct JsonHash {
        size_t operator()(const json& j) const noexcept {
            return std::hash<std::string>{}(j.dump());
        }
    };
    struct JsonEqual {
        bool operator()(const json& lhs, const json& rhs) const noexcept {
            return lhs == rhs;
        }
    };
    
    std::unordered_map<json, std::promise<Response>, JsonHash, JsonEqual> pendingRequests_;
    std::unordered_map<std::string, std::function<void(const json&)>> notificationHandlers_;
    std::function<void(double, const std::string&)> progressHandler_;
    std::function<void(const std::string&)> errorHandler_;
    std::string lastError_;
    uint64_t nextId_ = 1;
};

class JsonRpcServer {
public:
    explicit JsonRpcServer(std::unique_ptr<IpcTransport> transport);
    ~JsonRpcServer();

    bool start();
    void stop();

    // Register method handler
    template<typename Params, typename Result>
    void registerMethod(const std::string& method, std::function<Result(const Params&)> handler) {
        methodHandlers_[method] = [handler](const json& params) -> json {
            Params p = params.get<Params>();
            Result r = handler(p);
            return r;
        };
    }

    // Register notification handler
    void registerNotification(const std::string& method, std::function<void(const json&)> handler) {
        notificationHandlers_[method] = handler;
    }

    // Send notification to client
    void notify(const std::string& method, const json& params);

    // Send progress
    void sendProgress(double progress, const std::string& message = "");

    // Send error
    void sendError(const std::string& message, int code = -32000);

private:
    void handleMessage(const json& message);
    void handleRequest(const Request& request);
    void handleNotification(const Notification& notification);
    void handleTransportError(const std::string& error);
    Response makeErrorResponse(const json& id, int code, const std::string& message, const json& data = json{});

    std::unique_ptr<IpcTransport> transport_;
    std::atomic<bool> running_{false};
    std::unordered_map<std::string, std::function<json(const json&)>> methodHandlers_;
    std::unordered_map<std::string, std::function<void(const json&)>> notificationHandlers_;
};

} // namespace pdfpro::ipc