import QtQuick
import QtQuick.Controls

// Viewport - handles pan, zoom, tile-based rendering for large PDFs
Item {
    id: root
    anchors.fill: parent
    clip: true

    property double zoom: 1.0
    property int currentPage: 0
    property string currentTool: "pan"
    property rect visibleRect: Qt.rect(0, 0, width, height)

    signal zoomChanged(double zoom)
    signal pageChanged(int page)
    signal toolChanged(string tool)

    // Pan state
    property bool panning: false
    property point lastPanPos: Qt.point(0, 0)
    property point contentPos: Qt.point(0, 0)

    // Tile cache for progressive rendering
    property var tileCache: {}
    property int tileSize: 256

    // Background
    Rectangle {
        anchors.fill: parent
        color: "#2D2D2D" // Dark gray for page shadows
    }

    // Page container (transformed by zoom/pan)
    Item {
        id: pageContainer
        x: contentPos.x
        y: contentPos.y
        scale: root.zoom
        transformOrigin: Item.TopLeft
    }

    // Mouse handling
    MouseArea {
        anchors.fill: parent
        hoverEnabled: true
        drag.target: (root.currentTool === "pan") ? pageContainer : undefined
        drag.axis: Drag.XAndYAxis
        drag.minimumX: -(pageContainer.width * root.zoom - root.width)
        drag.maximumX: 0
        drag.minimumY: -(pageContainer.height * root.zoom - root.height)
        drag.maximumY: 0

        onPressed: {
            if (root.currentTool === "pan") {
                root.panning = true
                root.lastPanPos = Qt.point(mouse.x, mouse.y)
            } else if (root.currentTool === "zoom") {
                // Zoom to point
                var factor = mouse.modifiers & Qt.ControlModifier ? 0.8 : 1.25
                zoomAt(mouse.x, mouse.y, factor)
            }
        }
        onReleased: {
            root.panning = false
        }
        onPositionChanged: {
            if (root.panning) {
                root.contentPos = Qt.point(pageContainer.x, pageContainer.y)
                updateVisibleRect()
            }
        }

        // Wheel zoom
        wheelEnabled: true
        onWheel: {
            if (wheel.modifiers & Qt.ControlModifier) {
                var factor = wheel.angleDelta.y > 0 ? 1.15 : 0.87
                zoomAt(wheel.x, wheel.y, factor)
                wheel.accepted = true
            } else {
                // Pan vertically
                root.contentPos.y = Math.max(Math.min(root.contentPos.y - wheel.angleDelta.y, 0),
                    -(pageContainer.height * root.zoom - root.height))
                pageContainer.y = root.contentPos.y
                updateVisibleRect()
            }
        }
    }

    // Keyboard navigation
    Keys.onPressed: {
        switch (event.key) {
            case Qt.Key_PageUp: pageChanged(currentPage > 0 ? currentPage - 1 : 0); break
            case Qt.Key_PageDown: pageChanged(currentPage < root.pageCount - 1 ? currentPage + 1 : currentPage); break
            case Qt.Key_Home: pageChanged(0); break
            case Qt.Key_End: pageChanged(root.pageCount - 1); break
            case Qt.Key_Plus: case Qt.Key_Equal: zoomChanged(root.zoom * 1.2); break
            case Qt.Key_Minus: zoomChanged(root.zoom / 1.2); break
            case Qt.Key_0: zoomChanged(1.0); break
            case Qt.Key_Space: if (root.currentTool !== "pan") toolChanged("pan"); break
        }
    }

    function zoomAt(x, y, factor) {
        var newZoom = Math.max(0.1, Math.min(10.0, root.zoom * factor))
        if (newZoom === root.zoom) return

        // Calculate zoom center relative to content
        var contentX = (x - root.contentPos.x) / root.zoom
        var contentY = (y - root.contentPos.y) / root.zoom

        root.zoom = newZoom
        root.zoomChanged(newZoom)

        // Adjust content position to keep zoom center under cursor
        root.contentPos.x = x - contentX * newZoom
        root.contentPos.y = y - contentY * newZoom

        // Clamp
        root.contentPos.x = Math.max(Math.min(root.contentPos.x, 0), -(pageContainer.width * newZoom - root.width))
        root.contentPos.y = Math.max(Math.min(root.contentPos.y, 0), -(pageContainer.height * newZoom - root.height))

        pageContainer.x = root.contentPos.x
        pageContainer.y = root.contentPos.y
        pageContainer.scale = newZoom
        updateVisibleRect()
    }

    function updateVisibleRect() {
        var invZoom = 1.0 / root.zoom
        root.visibleRect = Qt.rect(
            -root.contentPos.x * invZoom,
            -root.contentPos.y * invZoom,
            root.width * invZoom,
            root.height * invZoom
        )
    }

    onZoomChanged: {
        pageContainer.scale = zoom
        updateVisibleRect()
    }

    onCurrentPageChanged: {
        // Page changed - reload tiles
        tileCache = {}
    }

    // Request tiles for visible area
    function requestTiles() {
        // TODO: Request tiles from Poppler engine via IPC
        // Engine returns base64 encoded tile images
        // Cache and display
    }
}