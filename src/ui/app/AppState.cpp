#include "AppState.h"
#include <QFileInfo>
#include <QDir>
#include <QCoreApplication>
#include <spdlog/spdlog.h>

namespace pdfpro::ui {

AppState::AppState(QObject* parent) : QObject(parent) {
    loadSettings();
}

AppState::~AppState() {
    stopEngines();
    saveSettings();
}

void AppState::loadSettings() {
    settings_.beginGroup("General");
    recentFiles_ = settings_.value("recentFiles", QStringList()).toStringList();
    darkMode_ = settings_.value("darkMode", false).toBool();
    defaultZoom_ = settings_.value("defaultZoom", 1.0).toDouble();
    lastDirectory_ = settings_.value("lastDirectory", QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation)).toString();
    sidebarVisible_ = settings_.value("sidebarVisible", true).toBool();
    sidebarTab_ = settings_.value("sidebarTab", 0).toInt();
    settings_.endGroup();

    // Clean up non-existent files
    QStringList validFiles;
    for (const QString& f : recentFiles_) {
        if (QFileInfo::exists(f)) validFiles << f;
    }
    recentFiles_ = validFiles;
}

void AppState::saveSettings() {
    settings_.beginGroup("General");
    settings_.setValue("recentFiles", recentFiles_);
    settings_.setValue("darkMode", darkMode_);
    settings_.setValue("defaultZoom", defaultZoom_);
    settings_.setValue("lastDirectory", lastDirectory_);
    settings_.setValue("sidebarVisible", sidebarVisible_);
    settings_.setValue("sidebarTab", sidebarTab_);
    settings_.endGroup();
}

QStringList AppState::recentFiles() const { return recentFiles_; }

void AppState::addRecentFile(const QString& filePath) {
    QString canonical = QFileInfo(filePath).canonicalFilePath();
    recentFiles_.removeAll(canonical);
    recentFiles_.prepend(canonical);
    if (recentFiles_.size() > 20) recentFiles_.resize(20);
    emit recentFilesChanged();
}

void AppState::removeRecentFile(const QString& filePath) {
    recentFiles_.removeAll(filePath);
    emit recentFilesChanged();
}

void AppState::clearRecentFiles() {
    recentFiles_.clear();
    emit recentFilesChanged();
}

bool AppState::darkMode() const { return darkMode_; }
void AppState::setDarkMode(bool enabled) {
    if (darkMode_ != enabled) {
        darkMode_ = enabled;
        emit darkModeChanged();
    }
}

double AppState::defaultZoom() const { return defaultZoom_; }
void AppState::setDefaultZoom(double zoom) {
    if (qFuzzyCompare(defaultZoom_, zoom)) return;
    defaultZoom_ = zoom;
    emit defaultZoomChanged();
}

QString AppState::lastDirectory() const { return lastDirectory_; }
void AppState::setLastDirectory(const QString& dir) {
    if (lastDirectory_ != dir) {
        lastDirectory_ = dir;
        emit lastDirectoryChanged();
    }
}

bool AppState::sidebarVisible() const { return sidebarVisible_; }
void AppState::setSidebarVisible(bool visible) {
    if (sidebarVisible_ != visible) {
        sidebarVisible_ = visible;
        emit sidebarVisibleChanged();
    }
}

int AppState::sidebarTab() const { return sidebarTab_; }
void AppState::setSidebarTab(int tab) {
    if (sidebarTab_ != tab) {
        sidebarTab_ = tab;
        emit sidebarTabChanged();
    }
}

bool AppState::startEngines() {
    if (popplerClient_ && qpdfClient_) return true;

    // Start Poppler engine process
    popplerProcess_ = std::make_unique<QProcess>();
    popplerProcess_->setProgram(QCoreApplication::applicationDirPath() + "/pdfpro-poppler-engine");
    popplerProcess_->start();
    if (!popplerProcess_->waitForStarted(5000)) {
        spdlog::error("Failed to start Poppler engine");
        return false;
    }

    auto popplerTransport = std::make_unique<StdioTransport>();
    // Note: For QProcess, we'd need a custom transport that uses QProcess channels
    // For now, using stdio transport assumes the engine is spawned as child with stdin/stdout connected
    // This is a simplification - in production, use QLocalSocket or named pipes on Windows

    popplerClient_ = std::make_unique<JsonRpcClient>(std::move(popplerTransport));
    if (!popplerClient_->connect()) {
        spdlog::error("Failed to connect to Poppler engine");
        return false;
    }

    // Start Qpdf engine process
    qpdfProcess_ = std::make_unique<QProcess>();
    qpdfProcess_->setProgram(QCoreApplication::applicationDirPath() + "/pdfpro-qpdf-engine");
    qpdfProcess_->start();
    if (!qpdfProcess_->waitForStarted(5000)) {
        spdlog::error("Failed to start Qpdf engine");
        return false;
    }

    auto qpdfTransport = std::make_unique<StdioTransport>();
    qpdfClient_ = std::make_unique<JsonRpcClient>(std::move(qpdfTransport));
    if (!qpdfClient_->connect()) {
        spdlog::error("Failed to connect to Qpdf engine");
        return false;
    }

    // Set up progress/error handlers
    popplerClient_->onProgress([this](double progress, const std::string& msg) {
        emit engineProgress(progress, QString::fromStdString(msg));
    });
    popplerClient_->onError([this](const std::string& err) {
        emit engineError(QString::fromStdString("Poppler: " + err));
    });
    qpdfClient_->onProgress([this](double progress, const std::string& msg) {
        emit engineProgress(progress, QString::fromStdString(msg));
    });
    qpdfClient_->onError([this](const std::string& err) {
        emit engineError(QString::fromStdString("Qpdf: " + err));
    });

    emit enginesStarted();
    return true;
}

void AppState::stopEngines() {
    if (popplerClient_) {
        popplerClient_->disconnect();
        popplerClient_.reset();
    }
    if (qpdfClient_) {
        qpdfClient_->disconnect();
        qpdfClient_.reset();
    }
    if (popplerProcess_) {
        popplerProcess_->terminate();
        popplerProcess_->waitForFinished(2000);
        popplerProcess_.reset();
    }
    if (qpdfProcess_) {
        qpdfProcess_->terminate();
        qpdfProcess_->waitForFinished(2000);
        qpdfProcess_.reset();
    }
    emit enginesStopped();
}

bool AppState::areEnginesRunning() const {
    return popplerClient_ && qpdfClient_ && popplerClient_->isConnected() && qpdfClient_->isConnected();
}

std::optional<AppState::Document> AppState::openDocument(const QString& filePath, const QString& password) {
    if (!popplerClient_) return std::nullopt;

    json params = {{"filePath", filePath.toStdString()}};
    if (!password.isEmpty()) params["password"] = password.toStdString();

    auto result = popplerClient_->call<DocumentHandle, DocumentHandle>(methods::OPEN_DOCUMENT, params);
    if (!result) return std::nullopt;

    DocumentHandle handle = *result;

    // Get document info
    auto infoResult = popplerClient_->call<DocumentInfo, DocumentInfo>(methods::GET_DOCUMENT_INFO, json{{"handle", handle}});
    if (!infoResult) {
        popplerClient_->notify(methods::CLOSE_DOCUMENT, json{{"handle", handle}});
        return std::nullopt;
    }

    Document doc;
    doc.handle = handle;
    doc.filePath = filePath;
    doc.info = *infoResult;
    doc.zoom = defaultZoom_;

    currentDoc_ = doc;
    addRecentFile(filePath);
    setLastDirectory(QFileInfo(filePath).absolutePath());

    emit documentOpened(handle);
    emit currentDocumentChanged(handle);

    return currentDoc_;
}

void AppState::closeDocument(DocumentHandle handle) {
    if (popplerClient_) {
        popplerClient_->notify(methods::CLOSE_DOCUMENT, json{{"handle", handle}});
    }
    if (currentDoc_ && currentDoc_->handle == handle) {
        currentDoc_.reset();
        emit currentDocumentChanged(DocumentHandle{});
    }
    emit documentClosed(handle);
}

void AppState::setCurrentDocument(DocumentHandle handle) {
    // Implementation would switch between multiple open documents
    if (currentDoc_ && currentDoc_->handle == handle) return;
    emit currentDocumentChanged(handle);
}

void AppState::pushUndo(const QString& action, std::function<void()> undoFn, std::function<void()> redoFn) {
    undoStack_.push_back({action, std::move(undoFn), std::move(redoFn)});
    if (undoStack_.size() > maxUndoStack_) undoStack_.erase(undoStack_.begin());
    redoStack_.clear();
    emit undoRedoChanged();
}

bool AppState::canUndo() const { return !undoStack_.empty(); }
bool AppState::canRedo() const { return !redoStack_.empty(); }

void AppState::undo() {
    if (undoStack_.empty()) return;
    auto entry = std::move(undoStack_.back());
    undoStack_.pop_back();
    entry.undoFn();
    redoStack_.push_back(std::move(entry));
    emit undoRedoChanged();
}

void AppState::redo() {
    if (redoStack_.empty()) return;
    auto entry = std::move(redoStack_.back());
    redoStack_.pop_back();
    entry.redoFn();
    undoStack_.push_back(std::move(entry));
    emit undoRedoChanged();
}

} // namespace pdfpro::ui