import QtQuick
import QtQuick.Controls

// Selection overlay - handles text/image/annotation selection
Item {
    id: root
    anchors.fill: parent
    property var document: null
    property int currentPage: 0
    property double zoom: 1.0
    property string tool: "pan"
    property rect viewportRect: Qt.rect(0, 0, width, height)
    signal selectionChanged(var selection)

    property point selectionStart: Qt.point(0, 0)
    property point selectionEnd: Qt.point(0, 0)
    property bool selecting: false

    // Selection rectangle
    Rectangle {
        id: selectionRect
        x: Math.min(root.selectionStart.x, root.selectionEnd.x)
        y: Math.min(root.selectionStart.y, root.selectionEnd.y)
        width: Math.abs(root.selectionEnd.x - root.selectionStart.x)
        height: Math.abs(root.selectionEnd.y - root.selectionStart.y)
        color: Material.primary
        opacity: 0.2
        border.color: Material.primary
        border.width: 1 / root.zoom
        visible: root.selecting && root.selectionStart !== root.selectionEnd
        radius: 2 / root.zoom
    }

    // Selection handles (for resizing)
    Repeater {
        model: root.selecting ? 8 : 0
        delegate: Rectangle {
            width: 10 / root.zoom
            height: 10 / root.zoom
            radius: 5 / root.zoom
            color: Material.primary
            border.color: "white"
            border.width: 1 / root.zoom
            x: {
                switch (index) {
                    case 0: return selectionRect.x - width/2 // top-left
                    case 1: return selectionRect.x + selectionRect.width/2 - width/2 // top-mid
                    case 2: return selectionRect.x + selectionRect.width - width/2 // top-right
                    case 3: return selectionRect.x + selectionRect.width - width/2 // mid-right
                    case 4: return selectionRect.x + selectionRect.width - width/2 // bottom-right
                    case 5: return selectionRect.x + selectionRect.width/2 - width/2 // bottom-mid
                    case 6: return selectionRect.x - width/2 // bottom-left
                    case 7: return selectionRect.x - width/2 // mid-left
                }
            }
            y: {
                switch (index) {
                    case 0: case 1: case 2: return selectionRect.y - height/2
                    case 3: return selectionRect.y + selectionRect.height/2 - height/2
                    case 4: case 5: case 6: return selectionRect.y + selectionRect.height - height/2
                    case 7: return selectionRect.y + selectionRect.height/2 - height/2
                }
            }
            MouseArea {
                anchors.fill: parent
                anchors.margins: -5 / root.zoom
                drag.target: parent
                drag.axis: (index % 2 === 0) ? Drag.XAndYAxis : (index < 3 || index > 5) ? Drag.YAxis : Drag.XAxis
                onDragStarted: root.selecting = true
            }
        }
    }

    // Highlight existing annotations/text selections
    // TODO: Render highlights from engine data

    // Mouse handling
    MouseArea {
        anchors.fill: parent
        hoverEnabled: true
        cursorShape: root.tool === "select" ? Qt.IBeamCursor : (root.tool === "highlight" ? Qt.CrossCursor : Qt.ArrowCursor)

        onPressed: {
            if (root.tool === "select" || root.tool === "highlight") {
                root.selectionStart = Qt.point(mouse.x, mouse.y)
                root.selectionEnd = Qt.point(mouse.x, mouse.y)
                root.selecting = true
            }
        }
        onPositionChanged: {
            if (root.selecting) {
                root.selectionEnd = Qt.point(mouse.x, mouse.y)
            }
        }
        onReleased: {
            if (root.selecting) {
                root.selecting = false
                var rect = Qt.rect(
                    Math.min(root.selectionStart.x, root.selectionEnd.x),
                    Math.min(root.selectionStart.y, root.selectionEnd.y),
                    Math.abs(root.selectionEnd.x - root.selectionStart.x),
                    Math.abs(root.selectionEnd.y - root.selectionStart.y)
                )
                if (rect.width > 5 && rect.height > 5) {
                    // Convert to PDF coordinates
                    var pdfRect = screenToPdf(rect)
                    if (root.tool === "select") {
                        // Request text in selection from engine
                        requestTextSelection(pdfRect)
                    } else if (root.tool === "highlight") {
                        // Add highlight annotation
                        addHighlightAnnotation(pdfRect)
                    }
                } else {
                    // Click - clear selection
                    root.selectionChanged(null)
                }
            }
        }
        onDoubleClicked: {
            if (root.tool === "select") {
                // Select word under cursor
                selectWordAt(mouse.x, mouse.y)
            }
        }
        onTripleClicked: {
            if (root.tool === "select") {
                // Select line under cursor
                selectLineAt(mouse.x, mouse.y)
            }
        }
    }

    function screenToPdf(screenRect) {
        // Convert screen coordinates to PDF page coordinates
        var pageItem = root.parent.pageView.pageRect
        var invZoom = 1.0 / root.zoom
        var pageX = (pageItem.x - root.viewportRect.x) * invZoom
        var pageY = (pageItem.y - root.viewportRect.y) * invZoom
        return Qt.rect(
            (screenRect.x - pageX) * invZoom,
            (screenRect.y - pageY) * invZoom,
            screenRect.width * invZoom,
            screenRect.height * invZoom
        )
    }

    function requestTextSelection(pdfRect) {
        // TODO: Call Poppler engine getTextInRect via IPC
        // For now, mock
        root.selectionChanged({ type: "text", rect: pdfRect, page: root.currentPage, text: "Selected text..." })
    }

    function addHighlightAnnotation(pdfRect) {
        // TODO: Call qpdf engine addAnnotation via IPC
        root.selectionChanged({ type: "highlight", rect: pdfRect, page: root.currentPage })
    }

    function selectWordAt(x, y) {
        // TODO: Request word bounds from engine
    }

    function selectLineAt(x, y) {
        // TODO: Request line bounds from engine
    }
}