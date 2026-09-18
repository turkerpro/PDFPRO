import QtQuick
import QtQuick.Controls

// Main PDF Viewer - orchestrates viewport, pages, selection
Item {
    id: root
    anchors.fill: parent
    property var document: null
    property int currentPage: 0
    property int pageCount: 0
    property double zoom: 1.0
    property string currentTool: "pan"
    signal toolChanged(string tool)

    // Viewport with pan/zoom
    Viewport {
        id: viewport
        anchors.fill: parent
        zoom: root.zoom
        currentPage: root.currentPage
        onZoomChanged: root.zoom = zoom
        onPageChanged: root.currentPage = page
        onToolChanged: root.toolChanged(tool)
    }

    // Page rendering layer
    PageView {
        id: pageView
        anchors.fill: parent
        document: root.document
        currentPage: root.currentPage
        zoom: root.zoom
        viewportRect: viewport.visibleRect
    }

    // Selection overlay
    SelectionLayer {
        id: selectionLayer
        anchors.fill: parent
        document: root.document
        currentPage: root.currentPage
        zoom: root.zoom
        tool: root.currentTool
        viewportRect: viewport.visibleRect
        onSelectionChanged: {
            if (selection) {
                contextBar.selection = selection
                contextBar.visible = true
            } else {
                contextBar.visible = false
            }
        }
    }

    // Context bar reference (from MainWindow)
    property var contextBar: null

    function goToPage(page) {
        if (page >= 0 && page < pageCount) {
            currentPage = page
            viewport.currentPage = page
        }
    }

    function zoomIn() {
        zoom = Math.min(zoom * 1.2, 10.0)
        viewport.zoom = zoom
    }

    function zoomOut() {
        zoom = Math.max(zoom / 1.2, 0.1)
        viewport.zoom = zoom
    }

    function resetZoom() {
        zoom = 1.0
        viewport.zoom = 1.0
    }

    function fitToWidth() {
        // TODO: Calculate zoom to fit page width
        zoom = 1.5 // placeholder
        viewport.zoom = zoom
    }

    function fitToPage() {
        // TODO: Calculate zoom to fit whole page
        zoom = 1.0 // placeholder
        viewport.zoom = zoom
    }

    onDocumentChanged: {
        if (document) {
            pageCount = document.info.pageCount
            currentPage = 0
            viewport.currentPage = 0
            // Load first page
        } else {
            pageCount = 0
            currentPage = 0
        }
    }
}