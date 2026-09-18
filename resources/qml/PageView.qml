import QtQuick
import QtQuick.Controls

// Page rendering - displays PDF pages via engine IPC
Item {
    id: root
    anchors.fill: parent
    property var document: null
    property int currentPage: 0
    property double zoom: 1.0
    property rect viewportRect: Qt.rect(0, 0, width, height)

    // Page background (shadow effect)
    Rectangle {
        id: pageShadow
        anchors.centerIn: parent
        width: pageRect.width + 20
        height: pageRect.height + 20
        color: "transparent"
        layer.enabled: true
        layer.effect: DropShadow {
            horizontalOffset: 0
            verticalOffset: 4
            radius: 12
            samples: 16
            color: "#00000080"
        }
    }

    // Page rect (paper)
    Rectangle {
        id: pageRect
        anchors.centerIn: parent
        width: 793.7 * root.zoom  // A4 width at 72 DPI
        height: 1122.5 * root.zoom // A4 height at 72 DPI
        color: "white"
        border.color: "#CCCCCC"
        border.width: 1 / root.zoom
        radius: 0
    }

    // Page content (rendered from engine)
    Image {
        id: pageImage
        anchors.fill: pageRect
        fillMode: Image.PreserveAspectFit
        asynchronous: true
        cache: false
        source: root.document && root.document.handle ? "image://pdfpage/" + root.document.handle + "/" + root.currentPage + "/" + (root.zoom * 150).toFixed(0) : ""
        onSourceChanged: {
            // Force reload when page/zoom changes
        }
    }

    // Loading indicator
    Rectangle {
        anchors.centerIn: pageRect
        width: 32
        height: 32
        radius: 16
        color: Material.primary
        opacity: pageImage.status === Image.Loading ? 0.2 : 0
        visible: pageImage.status === Image.Loading
        RotationAnimation on rotation { from: 0; to: 360; duration: 1000; running: visible; loops: Animation.Infinite }
    }

    // Page number label (when zoomed out)
    Text {
        anchors { bottom: pageRect.bottom; right: pageRect.right; margins: 8 }
        text: (root.currentPage + 1).toString()
        font.pixelSize: 12 / root.zoom
        font.bold: true
        color: Material.foreground
        opacity: root.zoom < 0.5 ? 0.7 : 0
        background: Rectangle {
            anchors.fill: parent
            anchors.margins: -4
            color: Material.background
            radius: 4
            opacity: 0.9
        }
    }

    // Update page rect when zoom changes
    onZoomChanged: {
        if (document && document.info) {
            var pageSize = document.info.pageSize
            pageRect.width = pageSize.w * zoom
            pageRect.height = pageSize.h * zoom
        }
    }

    onCurrentPageChanged: {
        if (document && document.info) {
            var pageSize = document.info.pageSize
            pageRect.width = pageSize.w * zoom
            pageRect.height = pageSize.h * zoom
        }
    }
}