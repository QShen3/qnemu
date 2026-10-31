import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

Item {
    id: page

    signal addGameRequested
    signal settingsRequested(string platform)

    /// Application palette passed from the main window so custom labels follow
    /// the system light/dark theme.
    property var theme

    function gameCountText() {
        const count = appController.library.gbCount + appController.library.gbcCount;
        if (count === 0) {
            return qsTr("No games in your library");
        }
        return count === 1 ? qsTr("1 game in your library") : qsTr("%1 games in your library").arg(count);
    }

    ColumnLayout {
        anchors.fill: parent
        anchors.leftMargin: 16
        anchors.rightMargin: 16
        anchors.topMargin: 14
        anchors.bottomMargin: 12
        spacing: 10

        RowLayout {
            Layout.fillWidth: true
            spacing: 4

            ColumnLayout {
                Layout.fillWidth: true
                spacing: 1

                Label {
                    Layout.fillWidth: true
                    text: qsTr("Game Library")
                    color: page.theme.windowText
                    font.pixelSize: 20
                    font.weight: Font.DemiBold
                    elide: Text.ElideRight
                }

                Label {
                    Layout.fillWidth: true
                    text: page.gameCountText() + (appController.paused ? qsTr("  ·  Paused") : "")
                    color: page.theme.mutedText
                    font.pixelSize: 11
                    elide: Text.ElideRight
                }
            }

            ToolButton {
                id: addButton

                text: "+"
                font.pixelSize: 18
                implicitWidth: 30
                implicitHeight: 30
                ToolTip.visible: hovered
                ToolTip.text: qsTr("Add a ROM to the library")
                onClicked: page.addGameRequested()
            }

            ToolButton {
                id: refreshButton

                text: "↻"
                font.pixelSize: 15
                implicitWidth: 30
                implicitHeight: 30
                ToolTip.visible: hovered
                ToolTip.text: qsTr("Rescan library folders")
                onClicked: appController.refreshLibrary()
            }
        }

        Rectangle {
            id: segmented

            property int currentIndex: 0

            Layout.fillWidth: true
            Layout.preferredHeight: 32
            radius: 8
            color: page.theme.withAlpha(page.theme.windowText, 0.06)

            RowLayout {
                anchors.fill: parent
                anchors.margins: 2
                spacing: 2

                Rectangle {
                    id: gbTab

                    Layout.fillWidth: true
                    Layout.fillHeight: true
                    radius: 6
                    color: segmented.currentIndex === 0 ? page.theme.accent : gbMouse.containsMouse ? page.theme.withAlpha(page.theme.windowText, 0.06) : "transparent"

                    Label {
                        anchors.centerIn: parent
                        text: qsTr("GB") + "  " + appController.library.gbCount
                        color: segmented.currentIndex === 0 ? page.theme.accentText : page.theme.mutedText
                        font.pixelSize: 12
                        font.weight: segmented.currentIndex === 0 ? Font.DemiBold : Font.Normal
                    }

                    MouseArea {
                        id: gbMouse
                        anchors.fill: parent
                        hoverEnabled: true
                        cursorShape: Qt.PointingHandCursor
                        onClicked: segmented.currentIndex = 0
                    }

                    Accessible.role: Accessible.RadioButton
                    Accessible.name: qsTr("GB games")
                    Accessible.checked: segmented.currentIndex === 0
                    Accessible.onPressAction: segmented.currentIndex = 0
                }

                Rectangle {
                    id: gbcTab

                    Layout.fillWidth: true
                    Layout.fillHeight: true
                    radius: 6
                    color: segmented.currentIndex === 1 ? page.theme.accent : gbcMouse.containsMouse ? page.theme.withAlpha(page.theme.windowText, 0.06) : "transparent"

                    Label {
                        anchors.centerIn: parent
                        text: qsTr("GBC") + "  " + appController.library.gbcCount
                        color: segmented.currentIndex === 1 ? page.theme.accentText : page.theme.mutedText
                        font.pixelSize: 12
                        font.weight: segmented.currentIndex === 1 ? Font.DemiBold : Font.Normal
                    }

                    MouseArea {
                        id: gbcMouse
                        anchors.fill: parent
                        hoverEnabled: true
                        cursorShape: Qt.PointingHandCursor
                        onClicked: segmented.currentIndex = 1
                    }

                    Accessible.role: Accessible.RadioButton
                    Accessible.name: qsTr("GBC games")
                    Accessible.checked: segmented.currentIndex === 1
                    Accessible.onPressAction: segmented.currentIndex = 1
                }
            }
        }

        StackLayout {
            Layout.fillWidth: true
            Layout.fillHeight: true
            currentIndex: segmented.currentIndex

            Item {
                ListView {
                    id: gbList
                    anchors.fill: parent
                    clip: true
                    spacing: 8
                    topMargin: 2
                    bottomMargin: 2
                    model: appController.library.gbGames
                    delegate: gameDelegate
                    ScrollBar.vertical: ScrollBar {}
                }

                EmptyLibrary {
                    anchors.centerIn: parent
                    width: Math.min(parent.width, 300)
                    height: implicitHeight
                    visible: gbList.count === 0
                    theme: page.theme
                    platformName: "GB"
                    onAddRequested: page.addGameRequested()
                    onSettingsRequested: page.settingsRequested("GB")
                }
            }

            Item {
                ListView {
                    id: gbcList
                    anchors.fill: parent
                    clip: true
                    spacing: 8
                    topMargin: 2
                    bottomMargin: 2
                    model: appController.library.gbcGames
                    delegate: gameDelegate
                    ScrollBar.vertical: ScrollBar {}
                }

                EmptyLibrary {
                    anchors.centerIn: parent
                    width: Math.min(parent.width, 300)
                    height: implicitHeight
                    visible: gbcList.count === 0
                    theme: page.theme
                    platformName: "GBC"
                    onAddRequested: page.addGameRequested()
                    onSettingsRequested: page.settingsRequested("GBC")
                }
            }
        }
    }

    Component {
        id: gameDelegate

        ItemDelegate {
            id: row

            width: ListView.view ? ListView.view.width : 0
            height: 60
            leftPadding: 12
            rightPadding: 6
            topPadding: 8
            bottomPadding: 8
            hoverEnabled: true
            opacity: model.missing ? 0.7 : 1.0

            background: Rectangle {
                radius: 10
                color: row.hovered ? page.theme.cardHover : page.theme.card
                border.width: 1
                border.color: row.hovered ? page.theme.withAlpha(page.theme.highlight, 0.45) : page.theme.border

                Behavior on color {
                    ColorAnimation {
                        duration: 120
                    }
                }
            }

            onClicked: {
                if (!model.missing) {
                    appController.openGame(model.path);
                }
            }

            contentItem: RowLayout {
                spacing: 10

                Rectangle {
                    Layout.preferredWidth: 40
                    Layout.preferredHeight: 22
                    radius: 5
                    color: model.platform === "GBC" ? page.theme.gbc : page.theme.gb

                    Label {
                        anchors.centerIn: parent
                        text: model.platform
                        color: model.platform === "GBC" ? page.theme.gbcText : page.theme.gbText
                        font.pixelSize: 10
                        font.bold: true
                        font.letterSpacing: 0.5
                    }
                }

                ColumnLayout {
                    Layout.fillWidth: true
                    spacing: 1

                    Label {
                        Layout.fillWidth: true
                        text: model.title
                        color: page.theme.windowText
                        font.pixelSize: 13
                        font.weight: Font.DemiBold
                        elide: Text.ElideRight
                    }

                    Label {
                        Layout.fillWidth: true
                        text: model.missing ? qsTr("File missing") : model.publisher + "  ·  " + model.region
                        color: model.missing ? page.theme.warning : page.theme.mutedText
                        font.pixelSize: 11
                        elide: Text.ElideRight
                    }
                }

                ToolButton {
                    id: rowMenuButton

                    text: "⋯"
                    implicitWidth: 28
                    implicitHeight: 28
                    opacity: row.hovered ? 1.0 : 0.55
                    onClicked: rowMenu.popup()
                    ToolTip.visible: hovered
                    ToolTip.text: qsTr("More actions")

                    Menu {
                        id: rowMenu

                        MenuItem {
                            text: qsTr("Remove from library")
                            onTriggered: appController.removeGame(model.path)
                        }
                    }
                }
            }
        }
    }
}
