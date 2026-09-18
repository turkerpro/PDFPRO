#pragma once

#include <QObject>
#include <QSettings>
#include <QStringList>
#include <QStandardPaths>
#include <QProcess>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <memory>
#include <vector>
#include <string>
#include <optional>
#include "../../core/ipc/IpcProtocol.h"
#include "../../core/ipc/IpcTransport.h"

namespace pdfpro::ui {

using namespace pdfpro::ipc;

class AppState : public QObject {
    Q_OBJECT
    Q_PROPERTY(QStringList recentFiles READ recentFiles NOTIFY recentFilesChanged)
    Q_PROPERTY(bool darkMode READ darkMode WRITE setDarkMode NOTIFY darkModeChanged)
    Q_PROPERTY(double defaultZoom READ defaultZoom WRITE setDefaultZoom NOTIFY defaultZoomChanged)
    Q_PROPERTY(QString lastDirectory READ lastDirectory WRITE setLastDirectory NOTIFY lastDirectoryChanged)
    Q_PROPERTY(bool sidebarVisible READ sidebarVisible WRITE setSidebarVisible NOTIFY sidebarVisibleChanged)
    Q_PROPERTY(int sidebarTab READ sidebarTab WRITE setSidebarTab NOTIFY sidebarTabChanged)

public:
    explicit AppState(QObject* parent = nullptr);
    ~AppState() override;

    // Settings
    QStringList recentFiles() const;
    void addRecentFile(const QString& filePath);
    void removeRecentFile(const QString& filePath);
    void clearRecentFiles();

    bool darkMode() const;
    void setDarkMode(bool enabled);

    double defaultZoom() const;
    void setDefaultZoom(double zoom);

    QString lastDirectory() const;
    void setLastDirectory(const QString& dir);

    bool sidebarVisible() const;
    void setSidebarVisible(bool visible);

    int sidebarTab() const; // 0=thumbnails, 1=bookmarks, 2=annotations, 3=search
    void setSidebarTab(int tab);

    // Engine management
    bool startEngines();
    void stopEngines();
    bool areEnginesRunning() const;

    JsonRpcClient* popplerClient() const { return popplerClient_.get(); }
    JsonRpcClient* qpdfClient() const { return qpdfClient_.get(); }

    // Document handling
    struct Document {
        DocumentHandle handle;
        QString filePath;
        DocumentInfo info;
        int currentPage = 0;
        double zoom = 1.0;
    };

    std::optional<Document> openDocument(const QString& filePath, const QString& password = "");
    void closeDocument(DocumentHandle handle);
    void setCurrentDocument(DocumentHandle handle);
    Document* currentDocument() { return currentDoc_ ? &*currentDoc_ : nullptr; }

    // Undo/Redo
    void pushUndo(const QString& action, std::function<void()> undoFn, std::function<void()> redoFn);
    bool canUndo() const;
    bool canRedo() const;
    void undo();
    void redo();

signals:
    void recentFilesChanged();
    void darkModeChanged();
    void defaultZoomChanged();
    void lastDirectoryChanged();
    void sidebarVisibleChanged();
    void sidebarTabChanged();
    void enginesStarted();
    void enginesStopped();
    void documentOpened(DocumentHandle handle);
    void documentClosed(DocumentHandle handle);
    void currentDocumentChanged(DocumentHandle handle);
    void undoRedoChanged();
    void engineProgress(double progress, const QString& message);
    void engineError(const QString& error);

private:
    void loadSettings();
    void saveSettings();
    QString settingsPath() const;

    QSettings settings_;
    QStringList recentFiles_;
    bool darkMode_ = false;
    double defaultZoom_ = 1.0;
    QString lastDirectory_;
    bool sidebarVisible_ = true;
    int sidebarTab_ = 0;

    std::unique_ptr<JsonRpcClient> popplerClient_;
    std::unique_ptr<JsonRpcClient> qpdfClient_;
    std::unique_ptr<QProcess> popplerProcess_;
    std::unique_ptr<QProcess> qpdfProcess_;

    std::optional<Document> currentDoc_;

    struct UndoEntry {
        QString action;
        std::function<void()> undoFn;
        std::function<void()> redoFn;
    };
    std::vector<UndoEntry> undoStack_;
    std::vector<UndoEntry> redoStack_;
    size_t maxUndoStack_ = 100;
};

} // namespace pdfpro::ui