import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

// Collapsible sidebar with tabs: Thumbnails, Bookmarks, Annotations, Search
Item {
    id: root
    width: 260
    property int currentTab: 0 // 0=thumbnails, 1=bookmarks, 2=annotations, 3=search
    signal currentTabChanged(int tab)

    Rectangle {
        anchors.fill: parent
        color: Material.background
        border.color: Material.foreground
        border.width: 0.5
        border.right: 1
    }

    Column {
        anchors.fill: parent

        // Tab bar
        Row {
            height: 36
            spacing: 0

            Repeater {
                model: ["thumbnails", "bookmarks", "annotations", "search"]
                delegate: ToolButton {
                    id: tabBtn
                    width: parent.width / 4
                    height: parent.height
                    checkable: true
                    checked: index === root.currentTab
                    exclusiveGroup: tabGroup
                    contentItem: Row {
                        spacing: 6
                        anchors.centerIn: parent
                        Image { source: "qrc:/icons/" + modelData + ".svg"; width: 16; height: 16; color: tabBtn.checked ? Material.primary : Material.foreground }
                        Text { text: qsTr(modelData); font.pixelSize: 11; visible: root.width > 200; color: tabBtn.checked ? Material.primary : Material.foreground }
                    }
                    onClicked: {
                        root.currentTab = index
                        root.currentTabChanged(index)
                    }
                }
            }
            ExclusiveGroup { id: tabGroup }
        }

        // Divider
        Rectangle { height: 1; width: parent.width; color: Material.foreground; opacity: 0.1 }

        // Tab content (StackView for smooth transitions)
        StackView {
            id: stackView
            anchors.fill: parent
            initialItem: thumbnailsTab
        }

        // Thumbnails Tab
        Component {
            id: thumbnailsTab
            ThumbnailStrip {
                anchors.fill: parent
                document: appState.currentDocument
                onPageClicked: viewer.goToPage(page)
            }
        }

        // Bookmarks Tab
        Component {
            id: bookmarksTab
            BookmarksPanel { anchors.fill: parent }
        }

        // Annotations Tab
        Component {
            id: annotationsTab
            AnnotationsPanel { anchors.fill: parent }
        }

        // Search Tab
        Component {
            id: searchTab
            SearchResultsPanel { anchors.fill: parent }
        }
    }
}