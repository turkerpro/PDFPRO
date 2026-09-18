import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

// Annotations panel
Item {
    id: root
    anchors.fill: parent

    Column {
        anchors.fill: parent

        ToolBar {
            height: 36
            RowLayout { anchors.fill: parent
                Label { text: qsTr("Annotations"); font.bold: true; Layout.fillWidth: true }
                ToolButton { icon.source: "qrc:/icons/filter.svg"; toolTip: "Filter"; onClicked: filterMenu.open() }
                Menu { id: filterMenu
                    MenuItem { text: "All"; checkable: true; checked: true }
                    MenuItem { text: "Highlights"; checkable: true }
                    MenuItem { text: "Comments"; checkable: true }
                    MenuItem { text: "Drawings"; checkable: true }
                }
            }
        }

        ListView {
            anchors.fill: parent
            model: annotationModel
            delegate: AnnotationDelegate {}
            ScrollBar.vertical: ScrollBar {}
        }

        ListModel { id: annotationModel }
    }

    Component {
        id: AnnotationDelegate
        Item {
            width: root.width
            height: 72
            Rectangle {
                anchors.fill: parent
                anchors.margins: 4
                color: ListView.isCurrentItem ? Material.primary : "transparent"
                opacity: ListView.isCurrentItem ? 0.1 : 0
                border.color: ListView.isCurrentItem ? Material.primary : "transparent"
                border.width: 1
                radius: 4
            }
            Row {
                anchors.fill: parent
                anchors.margins: 8
                spacing: 12
                Image {
                    width: 40; height: 40
                    source: "qrc:/icons/" + (model.type || "highlight") + ".svg"
                    color: model.color || Material.primary
                }
                Column {
                    spacing: 2
                    Text { text: model.title || qsTr("Annotation"); font.bold: true; font.pixelSize: 12; color: Material.foreground }
                    Text { text: qsTr("Page %1 • %2").arg(model.page + 1).arg(model.author || "Unknown"); font.pixelSize: 10; color: Material.foreground; opacity: 0.7 }
                    Text { text: model.contents || ""; font.pixelSize: 11; color: Material.foreground; elide: Text.ElideRight; width: root.width - 80 }
                }
            }
            MouseArea {
                anchors.fill: parent
                onClicked: viewer.goToPage(model.page)
                hoverEnabled: true
            }
        }
    }
}