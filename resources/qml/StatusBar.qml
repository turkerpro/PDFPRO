import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

// Status bar - page, zoom, cursor position
Rectangle {
    id: root
    height: 28
    color: Material.background
    border.color: Material.foreground
    border.width: 0.5
    border.top: 1

    RowLayout {
        anchors.fill: parent
        anchors.margins: 4
        spacing: 16

        // Page info
        Label {
            id: pageInfo
            text: appState.currentDocument ? "Page " + (viewer.currentPage + 1) + " of " + viewer.pageCount : "No document"
            font.pixelSize: 11
            color: Material.foreground
        }

        // Divider
        Rectangle { width: 1; height: 16; color: Material.foreground; opacity: 0.2; Layout.alignment: Qt.AlignVCenter }

        // Zoom
        Label {
            id: zoomInfo
            text: viewer ? (viewer.zoom * 100).toFixed(0) + "%" : "100%"
            font.pixelSize: 11
            color: Material.foreground
        }

        // Divider
        Rectangle { width: 1; height: 16; color: Material.foreground; opacity: 0.2; Layout.alignment: Qt.AlignVCenter }

        // Cursor position (in PDF points)
        Label {
            id: cursorPos
            text: "X: —  Y: —"
            font.pixelSize: 11
            color: Material.foreground
        }

        // Divider
        Rectangle { width: 1; height: 16; color: Material.foreground; opacity: 0.2; Layout.alignment: Qt.AlignVCenter }

        // Document info
        Label {
            id: docInfo
            text: appState.currentDocument ? (appState.currentDocument.info.pageSize.w | 0) + " × " + (appState.currentDocument.info.pageSize.h | 0) + " pt" : ""
            font.pixelSize: 11
            color: Material.foreground
            Layout.fillWidth: true
            horizontalAlignment: Text.AlignRight
        }

        // Tool indicator
        Label {
            id: toolInfo
            text: viewer ? viewer.currentTool.toUpperCase() : ""
            font.pixelSize: 11
            color: Material.primary
            font.bold: true
        }
    }
}