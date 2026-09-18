import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

// Context-sensitive toolbar appearing on selection
Item {
    id: root
    height: 40
    visible: false
    property var selection: null

    Rectangle {
        anchors.fill: parent
        color: Material.primary
        border.color: Material.foreground
        border.width: 0.5
        border.bottom: 1
        radius: 0
    }

    RowLayout {
        anchors.fill: parent
        anchors.margins: 8
        spacing: 8

        // Context label
        Label {
            id: contextLabel
            text: root.selection ? qsTr("%1 selected").arg(root.selection.type) : ""
            font.pixelSize: 12
            color: Material.primaryForeground
            font.bold: true
        }

        // Text selection actions
        Loader {
            id: textActions
            active: root.selection && (root.selection.type === "text" || root.selection.type === "word" || root.selection.type === "line")
            sourceComponent: textActionsComponent
        }

        Component {
            id: textActionsComponent
            RowLayout { spacing: 4
                ToolButton { icon.source: "qrc:/icons/highlighter.svg"; toolTip: "Highlight (Ctrl+Shift+H)"; onClicked: applyAnnotation("highlight") }
                ToolButton { icon.source: "qrc:/icons/underline.svg"; toolTip: "Underline"; onClicked: applyAnnotation("underline") }
                ToolButton { icon.source: "qrc:/icons/strikethrough.svg"; toolTip: "Strikethrough"; onClicked: applyAnnotation("strikeout") }
                ToolButton { icon.source: "qrc:/icons/copy.svg"; toolTip: "Copy (Ctrl+C)"; onClicked: copySelection() }
                ToolButton { icon.source: "qrc:/icons/search.svg"; toolTip: "Search Selection"; onClicked: searchSelection() }
            }
        }

        // Image/Object selection actions
        Loader {
            id: imageActions
            active: root.selection && root.selection.type === "image"
            sourceComponent: imageActionsComponent
        }

        Component {
            id: imageActionsComponent
            RowLayout { spacing: 4
                ToolButton { icon.source: "qrc:/icons/crop.svg"; toolTip: "Crop"; onClicked: cropImage() }
                ToolButton { icon.source: "qrc:/icons/rotate-cw.svg"; toolTip: "Rotate 90°"; onClicked: rotateImage(90) }
                ToolButton { icon.source: "qrc:/icons/rotate-ccw.svg"; toolTip: "Rotate -90°"; onClicked: rotateImage(-90) }
                ToolButton { icon.source: "qrc:/icons/copy.svg"; toolTip: "Copy"; onClicked: copySelection() }
                ToolButton { icon.source: "qrc:/icons/trash.svg"; toolTip: "Delete"; onClicked: deleteSelection() }
            }
        }

        // Annotation selection actions
        Loader {
            id: annotActions
            active: root.selection && root.selection.type === "annotation"
            sourceComponent: annotActionsComponent
        }

        Component {
            id: annotActionsComponent
            RowLayout { spacing: 4
                ToolButton { icon.source: "qrc:/icons/edit.svg"; toolTip: "Edit"; onClicked: editAnnotation() }
                ToolButton { icon.source: "qrc:/icons/color-swatch.svg"; toolTip: "Color"; onClicked: changeAnnotColor() }
                ToolButton { icon.source: "qrc:/icons/slider.svg"; toolTip: "Opacity"; onClicked: changeAnnotOpacity() }
                ToolButton { icon.source: "qrc:/icons/trash.svg"; toolTip: "Delete"; onClicked: deleteSelection() }
            }
        }

        Item { Layout.fillWidth: true }

        // Dismiss button
        ToolButton {
            icon.source: "qrc:/icons/x.svg"
            icon.width: 16
            icon.height: 16
            flat: true
            toolTip: "Dismiss (Esc)"
            onClicked: root.visible = false
        }
    }

    function applyAnnotation(type) {
        // TODO: Call viewer to add annotation
    }
    function copySelection() {
        // TODO: Copy to clipboard
    }
    function searchSelection() {
        // TODO: Open search with selection text
    }
    function cropImage() { }
    function rotateImage(deg) { }
    function deleteSelection() { root.visible = false }
    function editAnnotation() { }
    function changeAnnotColor() { }
    function changeAnnotOpacity() { }
}