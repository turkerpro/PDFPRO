import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

// Adaptive single-row toolbar — 5 primary tools + More menu
ToolBar {
    id: root
    height: 44
    padding: 4
    background: Rectangle {
        color: Material.background
        border.color: Material.foreground
        border.width: 0.5
        border.bottom: 1
    }

    RowLayout {
        anchors.fill: parent
        spacing: 4

        // File operations group
        ButtonGroup {
            id: fileGroup
        }

        ToolButton {
            id: openBtn
            text: qsTr("Open")
            icon.source: "qrc:/icons/folder-open.svg"
            icon.width: 18
            icon.height: 18
            toolTip: "Open PDF (Ctrl+O)"
            onClicked: {
                var w = Qt.createComponent("qrc:/qml/FileDialog.qml")
                if (w.status === Component.Ready) {
                    var dialog = w.createObject(root)
                    dialog.open()
                }
            }
            Layout.alignment: Qt.AlignLeft
        }

        ToolButton {
            id: saveBtn
            text: qsTr("Save")
            icon.source: "qrc:/icons/save.svg"
            icon.width: 18
            icon.height: 18
            enabled: appState.currentDocument !== null
            toolTip: "Save (Ctrl+S)"
            onClicked: saveCurrentDocument()
        }

        ToolButton {
            id: printBtn
            text: qsTr("Print")
            icon.source: "qrc:/icons/printer.svg"
            icon.width: 18
            icon.height: 18
            enabled: appState.currentDocument !== null
            toolTip: "Print (Ctrl+P)"
            onClicked: printDocument()
        }

        // Divider
        Rectangle { width: 1; height: 24; color: Material.foreground; opacity: 0.15; Layout.alignment: Qt.AlignVCenter }

        // Navigation group
        ToolButton {
            id: prevPageBtn
            icon.source: "qrc:/icons/chevron-left.svg"
            icon.width: 18
            icon.height: 18
            enabled: viewer && viewer.currentPage > 0
            toolTip: "Previous Page (PgUp)"
            onClicked: viewer.goToPage(viewer.currentPage - 1)
        }

        Label {
            id: pageLabel
            text: viewer ? (viewer.currentPage + 1) + " / " + viewer.pageCount : "—"
            font.pixelSize: 12
            color: Material.foreground
            Layout.alignment: Qt.AlignHCenter | Qt.AlignVCenter
            minimumWidth: 80
        }

        ToolButton {
            id: nextPageBtn
            icon.source: "qrc:/icons/chevron-right.svg"
            icon.width: 18
            icon.height: 18
            enabled: viewer && viewer.currentPage < viewer.pageCount - 1
            toolTip: "Next Page (PgDn)"
            onClicked: viewer.goToPage(viewer.currentPage + 1)
        }

        // Divider
        Rectangle { width: 1; height: 24; color: Material.foreground; opacity: 0.15; Layout.alignment: Qt.AlignVCenter }

        // Tool group (radio buttons - only one active)
        ExclusiveGroup { id: toolGroup }

        ToolButton {
            id: panTool
            checkable: true
            checked: root.currentTool === "pan"
            exclusiveGroup: toolGroup
            icon.source: "qrc:/icons/hand.svg"
            icon.width: 18
            icon.height: 18
            toolTip: "Pan (H)"
            onClicked: root.currentTool = "pan"
        }

        ToolButton {
            id: zoomTool
            checkable: true
            checked: root.currentTool === "zoom"
            exclusiveGroup: toolGroup
            icon.source: "qrc:/icons/zoom-in.svg"
            icon.width: 18
            icon.height: 18
            toolTip: "Zoom (Z)"
            onClicked: root.currentTool = "zoom"
        }

        ToolButton {
            id: selectTool
            checkable: true
            checked: root.currentTool === "select"
            exclusiveGroup: toolGroup
            icon.source: "qrc:/icons/cursor.svg"
            icon.width: 18
            icon.height: 18
            toolTip: "Select Text (V)"
            onClicked: root.currentTool = "select"
        }

        ToolButton {
            id: highlightTool
            checkable: true
            checked: root.currentTool === "highlight"
            exclusiveGroup: toolGroup
            icon.source: "qrc:/icons/highlighter.svg"
            icon.width: 18
            icon.height: 18
            toolTip: "Highlight (Ctrl+Shift+H)"
            onClicked: root.currentTool = "highlight"
        }

        // Divider
        Rectangle { width: 1; height: 24; color: Material.foreground; opacity: 0.15; Layout.alignment: Qt.AlignVCenter }

        // Zoom controls
        ToolButton {
            id: zoomOutBtn
            icon.source: "qrc:/icons/zoom-out.svg"
            icon.width: 18
            icon.height: 18
            enabled: viewer
            toolTip: "Zoom Out (Ctrl+-)"
            onClicked: viewer.zoomOut()
        }

        Label {
            id: zoomLabel
            text: viewer ? (viewer.zoom * 100).toFixed(0) + "%" : "100%"
            font.pixelSize: 12
            color: Material.foreground
            minimumWidth: 55
            horizontalAlignment: Text.AlignHCenter
        }

        ToolButton {
            id: zoomInBtn
            icon.source: "qrc:/icons/zoom-in.svg"
            icon.width: 18
            icon.height: 18
            enabled: viewer
            toolTip: "Zoom In (Ctrl+=)"
            onClicked: viewer.zoomIn()
        }

        ToolButton {
            id: fitWidthBtn
            text: "Fit Width"
            font.pixelSize: 11
            enabled: viewer
            toolTip: "Fit to Width (Ctrl+1)"
            onClicked: viewer.fitToWidth()
        }

        ToolButton {
            id: fitPageBtn
            text: "Fit Page"
            font.pixelSize: 11
            enabled: viewer
            toolTip: "Fit Page (Ctrl+2)"
            onClicked: viewer.fitToPage()
        }

        // Spacer
        Item { Layout.fillWidth: true }

        // More menu (overflow)
        MenuButton {
            id: moreBtn
            text: qsTr("More")
            icon.source: "qrc:/icons/more-horizontal.svg"
            icon.width: 18
            icon.height: 18
            menu: moreMenu
            popupMode: MenuButton.MenuButtonPopup
        }

        // Sidebar toggle
        ToolButton {
            id: sidebarToggle
            checkable: true
            checked: appState.sidebarVisible
            icon.source: "qrc:/icons/sidebar.svg"
            icon.width: 18
            icon.height: 18
            toolTip: "Toggle Sidebar (Ctrl+B)"
            onClicked: appState.sidebarVisible = checked
        }

        // Theme toggle
        ToolButton {
            id: themeToggle
            checkable: true
            checked: appState.darkMode
            icon.source: appState.darkMode ? "qrc:/icons/sun.svg" : "qrc:/icons/moon.svg"
            icon.width: 18
            icon.height: 18
            toolTip: "Toggle Theme"
            onClicked: appState.darkMode = checked
        }
    }
}