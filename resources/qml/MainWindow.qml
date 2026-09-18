import QtQuick
import QtQuick.Window
import QtQuick.Controls
import QtQuick.Controls.Material
import QtQuick.Layouts

// Main application window with Mica titlebar (Windows 11) and minimal chrome
Window {
    id: root
    width: 1280
    height: 800
    minimumWidth: 900
    minimumHeight: 600
    title: qsTr("PDFPRO")
    visibility: Window.Maximized

    // Material theme
    Material.theme: appState.darkMode ? Material.Dark : Material.Light
    Material.primary: "#0066CC"
    Material.accent: "#0066CC"
    Material.foreground: appState.darkMode ? "#FFFFFF" : "#000000"
    Material.background: appState.darkMode ? "#1E1E1E" : "#FFFFFF"

    // Windows 11 Mica backdrop
    property bool useMica: Qt.platform.os === "windows" && Qt.platform.windowsVersion >= Qt.platform.Windows11
    
    // Titlebar integration
    flags: Qt.Window | Qt.FramelessWindowHint
    // Note: For production, use a platform-specific implementation for Mica/Acrylic
    // This can be done via a QQuickItem subclass that calls DwmSetWindowAttribute

    property var currentTool: "pan" // pan, zoom, select, highlight
    property bool commandPaletteOpen: false

    // Keyboard shortcuts
    Shortcut { sequence: StandardKey.Open; onActivated: fileDialog.open() }
    Shortcut { sequence: StandardKey.Save; onActivated: saveCurrentDocument() }
    Shortcut { sequence: StandardKey.SaveAs; onActivated: saveCurrentDocumentAs() }
    Shortcut { sequence: "Ctrl+W"; onActivated: closeCurrentDocument() }
    Shortcut { sequence: "Ctrl+Q"; onActivated: Qt.quit() }
    Shortcut { sequence: "Ctrl+K"; onActivated: commandPalette.open() }
    Shortcut { sequence: "Ctrl+Shift+P"; onActivated: commandPalette.open() }
    Shortcut { sequence: "Ctrl+F"; onActivated: searchBar.focus = true }
    Shortcut { sequence: "Escape"; onActivated: handleEscape() }
    Shortcut { sequence: "Ctrl+Z"; onActivated: appState.undo() }
    Shortcut { sequence: "Ctrl+Y"; onActivated: appState.redo() }
    Shortcut { sequence: "Ctrl+Shift+Z"; onActivated: appState.redo() }
    Shortcut { sequence: "Ctrl+="; onActivated: zoomIn() }
    Shortcut { sequence: "Ctrl+-"; onActivated: zoomOut() }
    Shortcut { sequence: "Ctrl+0"; onActivated: resetZoom() }

    function handleEscape() {
        if (commandPaletteOpen) {
            commandPalette.close()
        } else if (searchBar.visible) {
            searchBar.visible = false
        } else if (contextBar.visible) {
            contextBar.visible = false
        }
    }

    function zoomIn() {
        if (viewer) viewer.zoomIn()
    }
    function zoomOut() {
        if (viewer) viewer.zoomOut()
    }
    function resetZoom() {
        if (viewer) viewer.resetZoom()
    }
    function saveCurrentDocument() {
        // TODO: Implement save via qpdf engine
    }
    function saveCurrentDocumentAs() {
        // TODO: Implement save as via qpdf engine
    }
    function closeCurrentDocument() {
        if (appState.currentDocument) {
            appState.closeDocument(appState.currentDocument.handle)
        }
    }

    // Custom titlebar (drag area)
    Rectangle {
        id: titleBar
        height: 36
        width: parent.width
        color: "transparent"
        z: 100

        // Drag region for frameless window
        MouseArea {
            anchors.fill: parent
            onPressed: root.startSystemMove()
            hoverEnabled: true
            cursorShape: Qt.ArrowCursor
        }

        RowLayout {
            anchors.fill: parent
            anchors.margins: 8

            // App icon
            Image {
                source: "qrc:/icons/app.svg"
                width: 20
                height: 20
                Layout.alignment: Qt.AlignVCenter
            }

            // Title
            Text {
                text: root.title + (appState.currentDocument ? " — " + appState.currentDocument.filePath : "")
                font.pixelSize: 12
                color: Material.foreground
                elide: Text.ElideRight
                Layout.fillWidth: true
                Layout.alignment: Qt.AlignVCenter
            }

            // Window controls
            Row {
                spacing: 0
                Layout.alignment: Qt.AlignRight | Qt.AlignVCenter

                Button {
                    id: minimizeBtn
                    width: 40
                    height: 28
                    flat: true
                    contentItem: Image { source: "qrc:/icons/minimize.svg"; width: 12; height: 12 }
                    onClicked: root.showMinimized()
                    ToolTip.text: "Minimize (Ctrl+M)"
                }
                Button {
                    id: maximizeBtn
                    width: 40
                    height: 28
                    flat: true
                    contentItem: Image { source: root.visibility === Window.Maximized ? "qrc:/icons/restore.svg" : "qrc:/icons/maximize.svg"; width: 12; height: 12 }
                    onClicked: root.visibility = root.visibility === Window.Maximized ? Window.Windowed : Window.Maximized
                    ToolTip.text: "Maximize/Restore"
                }
                Button {
                    id: closeBtn
                    width: 40
                    height: 28
                    flat: true
                    contentItem: Image { source: "qrc:/icons/close.svg"; width: 12; height: 12 }
                    onClicked: root.close()
                    ToolTip.text: "Close (Ctrl+Q)"
                    background: Rectangle {
                        color: closeBtn.hovered ? "#E81123" : "transparent"
                        radius: 0
                    }
                }
            }
        }
    }

    // Main content area
    Column {
        anchors.fill: parent
        anchors.topMargin: titleBar.height

        // Adaptive Toolbar
        ToolBar {
            id: toolBar
            visible: appState.currentDocument !== null
        }

        // Context Bar (appears on selection)
        ContextBar {
            id: contextBar
            visible: false
        }

        // Search Bar (Ctrl+F)
        SearchBar {
            id: searchBar
            visible: false
            width: parent.width
        }

        // Main split view: Sidebar + Viewer
        Row {
            id: mainSplit
            anchors.fill: parent

            // Sidebar (collapsible)
            Sidebar {
                id: sidebar
                visible: appState.sidebarVisible && appState.currentDocument !== null
                currentTab: appState.sidebarTab
                onCurrentTabChanged: appState.sidebarTab = currentTab
            }

            // Vertical divider
            Rectangle {
                id: sidebarDivider
                width: 1
                color: Material.foreground
                opacity: 0.1
                visible: sidebar.visible
            }

            // PDF Viewer (main area)
            PDFViewer {
                id: viewer
                anchors.fill: parent
                currentTool: root.currentTool
                onToolChanged: root.currentTool = tool
            }
        }

        // Status Bar
        StatusBar {
            id: statusBar
            visible: appState.currentDocument !== null
        }
    }

    // Command Palette (Ctrl+K)
    CommandPalette {
        id: commandPalette
        onActionTriggered: executeCommand(action)
    }

    // File Dialog
    FileDialog {
        id: fileDialog
        title: "Open PDF"
        nameFilters: ["PDF Files (*.pdf)", "All Files (*)"]
        selectExisting: true
        onAccepted: {
            if (fileDialog.fileUrls.length > 0) {
                appState.openDocument(fileDialog.fileUrls[0].toLocalFile())
            }
        }
    }

    // Recent Files Menu (for toolbar)
    Menu {
        id: recentFilesMenu
        Instantiator {
            model: appState.recentFiles
            delegate: MenuItem {
                text: modelData
                onTriggered: appState.openDocument(modelData)
            }
        }
    }

    // Toolbar More Menu
    Menu {
        id: moreMenu
        MenuItem { text: "Rotate Left"; shortcut: "Ctrl+["; onTriggered: rotatePage(-90) }
        MenuItem { text: "Rotate Right"; shortcut: "Ctrl+]"; onTriggered: rotatePage(90) }
        MenuItem { text: "Delete Page"; shortcut: "Del"; onTriggered: deletePage() }
        MenuSeparator { }
        MenuItem { text: "Extract Pages..."; onTriggered: extractPages() }
        MenuItem { text: "Insert Pages..."; onTriggered: insertPages() }
        MenuSeparator { }
        MenuItem { text: "OCR Current Page"; shortcut: "Ctrl+Shift+O"; onTriggered: ocrPage() }
        MenuItem { text: "Redact..."; onTriggered: redactTool() }
        MenuSeparator { }
        MenuItem { text: "Properties..."; shortcut: "Ctrl+D"; onTriggered: showProperties() }
    }
}