import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

// Bookmarks panel (placeholder)
Item {
    id: root
    anchors.fill: parent

    Column {
        anchors.fill: parent
        spacing: 0

        // Toolbar
        ToolBar {
            height: 36
            RowLayout { anchors.fill: parent
                Label { text: qsTr("Bookmarks"); font.bold: true; Layout.fillWidth: true }
                ToolButton { icon.source: "qrc:/icons/add.svg"; toolTip: "Add Bookmark"; onClicked: addBookmark() }
                ToolButton { icon.source: "qrc:/icons/folder-plus.svg"; toolTip: "Add Folder"; onClicked: addFolder() }
            }
        }

        // Tree view
        TreeView {
            anchors.fill: parent
            model: bookmarkModel
            TableViewColumn { title: "Title"; role: "title"; width: root.width - 40 }
            TableViewColumn { title: "Page"; role: "page"; width: 60 }
        }

        ListModel { id: bookmarkModel }
    }

    function addBookmark() { }
    function addFolder() { }
}