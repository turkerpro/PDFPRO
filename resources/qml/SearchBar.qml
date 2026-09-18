import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

// Search bar (Ctrl+F) - inline in toolbar area
Rectangle {
    id: root
    height: 40
    visible: false
    property var searchResults: []
    property int currentResultIndex: -1

    color: Material.background
    border.color: Material.foreground
    border.width: 0.5
    border.top: 1

    RowLayout {
        anchors.fill: parent
        anchors.margins: 8
        spacing: 8

        // Search icon
        Image { source: "qrc:/icons/search.svg"; width: 18; height: 18; color: Material.foreground; opacity: 0.6 }

        // Search input
        TextField {
            id: searchInput
            Layout.fillWidth: true
            placeholderText: qsTr("Search in document...")
            font.pixelSize: 13
            background: Rectangle {
                radius: 4
                color: Material.background
                border.color: Material.foreground
                border.width: 1
            }
            onAccepted: performSearch()
            Keys.onEscapePressed: root.visible = false
        }

        // Options
        ToolButton {
            id: caseSensitiveBtn
            checkable: true
            checked: false
            icon.source: "qrc:/icons/case-sensitive.svg"
            icon.width: 16
            icon.height: 16
            toolTip: "Match Case (Alt+C)"
            onClicked: performSearch()
        }
        ToolButton {
            id: wholeWordBtn
            checkable: true
            checked: false
            icon.source: "qrc:/icons/type.svg"
            icon.width: 16
            icon.height: 16
            toolTip: "Whole Word (Alt+W)"
            onClicked: performSearch()
        }
        ToolButton {
            id: regexBtn
            checkable: true
            checked: false
            icon.source: "qrc:/icons/regex.svg"
            icon.width: 16
            icon.height: 16
            toolTip: "Regular Expression (Alt+R)"
            onClicked: performSearch()
        }

        // Navigation
        ToolButton {
            id: prevResultBtn
            icon.source: "qrc:/icons/chevron-up.svg"
            icon.width: 18
            icon.height: 18
            enabled: root.currentResultIndex > 0
            toolTip: "Previous Result (Shift+Enter)"
            onClicked: gotoResult(root.currentResultIndex - 1)
        }
        ToolButton {
            id: nextResultBtn
            icon.source: "qrc:/icons/chevron-down.svg"
            icon.width: 18
            icon.height: 18
            enabled: root.currentResultIndex < root.searchResults.length - 1
            toolTip: "Next Result (Enter)"
            onClicked: gotoResult(root.currentResultIndex + 1)
        }

        Label {
            id: resultCountLabel
            text: root.searchResults.length > 0 ? (root.currentResultIndex + 1) + " / " + root.searchResults.length : ""
            font.pixelSize: 12
            color: Material.foreground
        }

        // Close
        ToolButton {
            icon.source: "qrc:/icons/x.svg"
            icon.width: 16
            icon.height: 16
            flat: true
            toolTip: "Close (Esc)"
            onClicked: root.visible = false
        }
    }

    function performSearch() {
        if (!searchInput.text || !appState.currentDocument) return
        // TODO: Call Poppler engine searchText
        // For now, mock results
        root.searchResults = [
            { page: 0, text: searchInput.text, bounds: { x: 100, y: 100, w: 200, h: 20 } },
            { page: 2, text: searchInput.text, bounds: { x: 150, y: 300, w: 180, h: 20 } }
        ]
        root.currentResultIndex = 0
        if (root.searchResults.length > 0) {
            gotoResult(0)
        }
    }

    function gotoResult(index) {
        if (index < 0 || index >= root.searchResults.length) return
        root.currentResultIndex = index
        var result = root.searchResults[index]
        // TODO: viewer.goToPage(result.page) and highlight
    }
}