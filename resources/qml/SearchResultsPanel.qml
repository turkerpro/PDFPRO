import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

// Search results panel
Item {
    id: root
    anchors.fill: parent
    property var searchResults: []

    Column {
        anchors.fill: parent

        ToolBar {
            height: 36
            RowLayout { anchors.fill: parent
                Label { text: qsTr("Search Results"); font.bold: true; Layout.fillWidth: true }
                ToolButton { icon.source: "qrc:/icons/x.svg"; toolTip: "Clear"; onClicked: root.searchResults = [] }
            }
        }

        ListView {
            anchors.fill: parent
            model: root.searchResults
            delegate: SearchResultDelegate {}
            ScrollBar.vertical: ScrollBar {}
        }
    }

    Component {
        id: SearchResultDelegate
        Item {
            width: root.width
            height: 64
            Rectangle {
                anchors.fill: parent
                anchors.margins: 4
                color: ListView.isCurrentItem ? Material.primary : "transparent"
                opacity: ListView.isCurrentItem ? 0.1 : 0
                border.color: ListView.isCurrentItem ? Material.primary : "transparent"
                border.width: 1
                radius: 4
            }
            Column {
                anchors.fill: parent
                anchors.margins: 8
                spacing: 4
                Row {
                    spacing: 8
                    Text { text: qsTr("Page %1").arg(model.page + 1); font.bold: true; font.pixelSize: 11; color: Material.primary }
                    Text { text: model.matchText; font.pixelSize: 11; color: Material.foreground }
                }
                Text {
                    text: model.contextBefore + " <b>" + model.matchText + "</b> " + model.contextAfter
                    font.pixelSize: 11
                    color: Material.foreground
                    elide: Text.ElideRight
                    width: root.width - 16
                }
            }
            MouseArea {
                anchors.fill: parent
                onClicked: {
                    viewer.goToPage(model.page)
                    // TODO: Highlight match
                }
                hoverEnabled: true
            }
        }
    }
}