import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

// Command Palette - Ctrl+K / Ctrl+Shift+P
Popup {
    id: root
    width: 600
    height: 400
    x: (parent.width - width) / 2
    y: 80
    modal: true
    focus: true
    closePolicy: Popup.CloseOnEscape | Popup.CloseOnPressOutside
    property var actions: []
    signal actionTriggered(var action)

    enter: Transition {
        SequentialAnimation {
            PropertyAnimation { property: "opacity"; from: 0; to: 1; duration: 120; easing.type: Easing.OutQuad }
            PropertyAnimation { property: "y"; from: 60; to: 80; duration: 120; easing.type: Easing.OutQuad }
        }
    }
    exit: Transition {
        ParallelAnimation {
            PropertyAnimation { property: "opacity"; to: 0; duration: 100; easing.type: Easing.InQuad }
            PropertyAnimation { property: "y"; to: 60; duration: 100; easing.type: Easing.InQuad }
        }
    }

    background: Rectangle {
        radius: 8
        color: Material.background
        border.color: Material.primary
        border.width: 1
        layer.enabled: true
        layer.effect: DropShadow {
            horizontalOffset: 0
            verticalOffset: 8
            radius: 24
            samples: 16
            color: "#00000060"
        }
    }

    Column {
        anchors.fill: parent

        // Search input
        TextField {
            id: searchInput
            width: parent.width
            height: 48
            placeholderText: qsTr("Type a command or search...")
            font.pixelSize: 14
            background: Rectangle {
                color: Material.background
                border.color: Material.foreground
                border.width: 0
            }
            leftPadding: 16
            rightPadding: 16
            onTextChanged: filterModel()
            Keys.onEscapePressed: root.close()
            Keys.onReturnPressed: {
                if (listView.count > 0) {
                    listView.currentIndex = 0
                    triggerAction(listView.model.get(0))
                }
            }
            Keys.onDownPressed: {
                if (listView.currentIndex < listView.count - 1) listView.currentIndex++
            }
            Keys.onUpPressed: {
                if (listView.currentIndex > 0) listView.currentIndex--
            }
        }

        // Divider
        Rectangle { height: 1; width: parent.width; color: Material.foreground; opacity: 0.1 }

        // Results list
        ListView {
            id: listView
            anchors.fill: parent
            model: filteredModel
            currentIndex: -1
            highlightFollowsCurrentItem: true
            highlight: Rectangle {
                color: Material.primary
                opacity: 0.15
                radius: 4
                x: 4; y: 2
                width: parent.width - 8
                height: 40
            }
            delegate: ItemDelegate {
                width: parent.width
                height: 40
                leftPadding: 16
                rightPadding: 16
                highlighted: ListView.isCurrentItem

                Row {
                    anchors.fill: parent
                    spacing: 12
                    Image { source: "qrc:/icons/" + (model.icon || "command") + ".svg"; width: 18; height: 18; color: Material.foreground; opacity: 0.7 }
                    Column {
                        spacing: 2
                        Text { text: model.title; font.pixelSize: 13; color: Material.foreground }
                        Text { text: model.description; font.pixelSize: 11; color: Material.foreground; opacity: 0.6 }
                    }
                    Item { Layout.fillWidth: true }
                    Text {
                        text: model.shortcut
                        font.pixelSize: 10
                        font.family: "JetBrains Mono"
                        color: Material.foreground
                        opacity: 0.5
                    }
                }
                onClicked: root.triggerAction(model)
            }
            ScrollBar.vertical: ScrollBar { width: 6 }
        }
    }

    function open() {
        searchInput.text = ""
        searchInput.forceActiveFocus()
        listView.currentIndex = -1
        root.open()
    }

    function close() {
        searchInput.text = ""
        root.close()
    }

    function filterModel() {
        // Simple filter
        var query = searchInput.text.toLowerCase()
        filteredModel.clear()
        for (var i = 0; i < root.actions.length; i++) {
            var a = root.actions[i]
            if (!query || a.title.toLowerCase().includes(query) ||
                a.description.toLowerCase().includes(query) ||
                (a.shortcut && a.shortcut.toLowerCase().includes(query))) {
                filteredModel.append(a)
            }
        }
        if (filteredModel.count > 0) listView.currentIndex = 0
    }

    function triggerAction(action) {
        root.actionTriggered(action)
        root.close()
    }

    ListModel { id: filteredModel }

    // Populate actions from app
    Component.onCompleted: {
        var allActions = [
            // File
            { title: "Open File", description: "Open a PDF document", shortcut: "Ctrl+O", icon: "folder-open", action: "file.open" },
            { title: "Save", description: "Save current document", shortcut: "Ctrl+S", icon: "save", action: "file.save" },
            { title: "Save As", description: "Save document with new name", shortcut: "Ctrl+Shift+S", icon: "save-as", action: "file.saveAs" },
            { title: "Print", description: "Print document", shortcut: "Ctrl+P", icon: "printer", action: "file.print" },
            { title: "Close Document", description: "Close current document", shortcut: "Ctrl+W", icon: "x", action: "file.close" },

            // Edit
            { title: "Undo", description: "Undo last action", shortcut: "Ctrl+Z", icon: "undo", action: "edit.undo" },
            { title: "Redo", description: "Redo last undone action", shortcut: "Ctrl+Y", icon: "redo", action: "edit.redo" },
            { title: "Copy", description: "Copy selection", shortcut: "Ctrl+C", icon: "copy", action: "edit.copy" },
            { title: "Select All", description: "Select all text on page", shortcut: "Ctrl+A", icon: "select-all", action: "edit.selectAll" },
            { title: "Find", description: "Find in document", shortcut: "Ctrl+F", icon: "search", action: "edit.find" },
            { title: "Find Next", description: "Find next occurrence", shortcut: "F3", icon: "chevron-down", action: "edit.findNext" },
            { title: "Find Previous", description: "Find previous occurrence", shortcut: "Shift+F3", icon: "chevron-up", action: "edit.findPrev" },

            // View
            { title: "Zoom In", description: "Increase zoom level", shortcut: "Ctrl+=", icon: "zoom-in", action: "view.zoomIn" },
            { title: "Zoom Out", description: "Decrease zoom level", shortcut: "Ctrl+-", icon: "zoom-out", action: "view.zoomOut" },
            { title: "Actual Size", description: "Reset zoom to 100%", shortcut: "Ctrl+0", icon: "maximize", action: "view.resetZoom" },
            { title: "Fit to Width", description: "Fit page to window width", shortcut: "Ctrl+1", icon: "maximize-2", action: "view.fitWidth" },
            { title: "Fit to Page", description: "Fit entire page in window", shortcut: "Ctrl+2", icon: "minimize-2", action: "view.fitPage" },
            { title: "Toggle Sidebar", description: "Show/hide sidebar", shortcut: "Ctrl+B", icon: "sidebar", action: "view.toggleSidebar" },
            { title: "Toggle Dark Mode", description: "Switch between light/dark theme", shortcut: "", icon: "moon", action: "view.toggleTheme" },

            // Navigation
            { title: "First Page", description: "Go to first page", shortcut: "Home", icon: "skip-back", action: "nav.first" },
            { title: "Previous Page", description: "Go to previous page", shortcut: "Page Up", icon: "chevron-left", action: "nav.prev" },
            { title: "Next Page", description: "Go to next page", shortcut: "Page Down", icon: "chevron-right", action: "nav.next" },
            { title: "Last Page", description: "Go to last page", shortcut: "End", icon: "skip-forward", action: "nav.last" },
            { title: "Go to Page", description: "Jump to specific page", shortcut: "Ctrl+G", icon: "navigation", action: "nav.goto" },

            // Tools
            { title: "Pan Tool", description: "Pan document (hand tool)", shortcut: "H", icon: "hand", action: "tool.pan" },
            { title: "Zoom Tool", description: "Zoom tool", shortcut: "Z", icon: "zoom-in", action: "tool.zoom" },
            { title: "Select Tool", description: "Select text tool", shortcut: "V", icon: "cursor", action: "tool.select" },
            { title: "Highlight Tool", description: "Highlight text", shortcut: "Ctrl+Shift+H", icon: "highlighter", action: "tool.highlight" },

            // Page operations
            { title: "Rotate Left", description: "Rotate page 90° counter-clockwise", shortcut: "Ctrl+[", icon: "rotate-ccw", action: "page.rotateLeft" },
            { title: "Rotate Right", description: "Rotate page 90° clockwise", shortcut: "Ctrl+]", icon: "rotate-cw", action: "page.rotateRight" },
            { title: "Delete Page", description: "Delete current page", shortcut: "Delete", icon: "trash", action: "page.delete" },
            { title: "Insert Page", description: "Insert blank page", shortcut: "Ctrl+Shift+N", icon: "plus-square", action: "page.insert" },
            { title: "Extract Pages", description: "Extract pages to new PDF", shortcut: "", icon: "copy", action: "page.extract" },

            // Annotations
            { title: "Add Highlight", description: "Highlight selected text", shortcut: "Ctrl+Shift+H", icon: "highlighter", action: "annot.highlight" },
            { title: "Add Underline", description: "Underline selected text", shortcut: "", icon: "underline", action: "annot.underline" },
            { title: "Add Strikethrough", description: "Strikethrough selected text", shortcut: "", icon: "strikethrough", action: "annot.strikeout" },
            { title: "Add Comment", description: "Add text comment", shortcut: "Ctrl+Alt+M", icon: "message-square", action: "annot.comment" },
            { title: "Add Drawing", description: "Freehand drawing", shortcut: "Ctrl+Shift+D", icon: "pen-tool", action: "annot.draw" },

            // Advanced
            { title: "OCR Current Page", description: "Recognize text on current page", shortcut: "Ctrl+Shift+O", icon: "scan-text", action: "advanced.ocr" },
            { title: "Redact", description: "Permanently remove content", shortcut: "", icon: "eye-off", action: "advanced.redact" },
            { title: "Merge Documents", description: "Combine multiple PDFs", shortcut: "", icon: "merge", action: "advanced.merge" },
            { title: "Split Document", description: "Split PDF into multiple files", shortcut: "", icon: "scissors", action: "advanced.split" },
            { title: "Export as Images", description: "Export pages as image files", shortcut: "", icon: "image", action: "advanced.exportImages" },
            { title: "Document Properties", description: "View document metadata", shortcut: "Ctrl+D", icon: "file-text", action: "advanced.properties" },
            { title: "Settings", description: "Open application settings", shortcut: "Ctrl+,", icon: "settings", action: "settings" },
        ]
        root.actions = allActions
        filterModel()
    }
}