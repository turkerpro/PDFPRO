import QtQuick
import QtQuick.Controls

// Virtualized thumbnail strip - lazy loads thumbnails
ListView {
    id: root
    anchors.fill: parent
    clip: true
    orientation: ListView.Vertical
    spacing: 8
    cacheBuffer: 200
    model: thumbnailModel

    property var document: null
    signal pageClicked(int page)

    ListModel { id: thumbnailModel }

    Component.onCompleted: {
        if (document) loadThumbnails()
    }

    onDocumentChanged: {
        thumbnailModel.clear()
        if (document) loadThumbnails()
    }

    function loadThumbnails() {
        if (!document || !document.handle) return
        // TODO: Call Poppler engine getThumbnails
        // Mock for now
        for (var i = 0; i < document.info.pageCount; i++) {
            thumbnailModel.append({ page: i, loaded: false })
        }
    }

    delegate: Item {
        id: delegateRoot
        width: root.width - 16
        height: 120

        Rectangle {
            anchors.fill: parent
            anchors.margins: 4
            radius: 4
            color: Material.background
            border.color: ListView.isCurrentItem ? Material.primary : Material.foreground
            border.width: ListView.isCurrentItem ? 2 : 1
            opacity: model.loaded ? 1 : 0.5

            Image {
                id: thumbImg
                anchors.centerIn: parent
                width: Math.min(parent.width - 8, (parent.height - 8) * (model.thumbWidth / model.thumbHeight))
                height: Math.min(parent.height - 8, (parent.width - 8) * (model.thumbHeight / model.thumbWidth))
                source: model.thumbData ? "data:image/png;base64," + model.thumbData : ""
                fillMode: Image.PreserveAspectFit
                asynchronous: true
                visible: model.loaded
            }

            // Loading placeholder
            Rectangle {
                anchors.centerIn: parent
                width: 40
                height: 40
                radius: 20
                color: Material.primary
                opacity: 0.2
                visible: !model.loaded
                RotationAnimation on rotation { from: 0; to: 360; duration: 1000; running: !model.loaded; loops: Animation.Infinite }
            }

            // Page number badge
            Text {
                anchors { bottom: parent.bottom; right: parent.right; margins: 4 }
                text: model.page + 1
                font.pixelSize: 11
                font.bold: true
                color: Material.foreground
                background: Rectangle {
                    anchors.fill: parent
                    anchors.margins: -2
                    color: Material.background
                    radius: 3
                    opacity: 0.9
                }
            }
        }

        MouseArea {
            anchors.fill: parent
            onClicked: root.pageClicked(model.page)
            hoverEnabled: true
        }

        // Load thumbnail on demand
        Loader {
            id: thumbLoader
            active: ListView.isCurrentItem || (index >= root.contentY / root.height - 1 && index <= (root.contentY + root.height) / root.height + 1)
            sourceComponent: thumbLoadComponent
            onStatusChanged: {
                if (status === Loader.Ready && item) {
                    model.thumbData = item.thumbData
                    model.thumbWidth = item.thumbWidth
                    model.thumbHeight = item.thumbHeight
                    model.loaded = true
                }
            }
        }

        Component {
            id: thumbLoadComponent
            Item {
                property string thumbData: ""
                property int thumbWidth: 100
                property int thumbHeight: 100
                // Actual loading happens in C++ via engine
                // This is a placeholder - real implementation calls getThumbnail via IPC
            }
        }
    }

    ScrollBar.vertical: ScrollBar { width: 8; policy: ScrollBar.AlwaysOn }
}