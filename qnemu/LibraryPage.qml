import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

Item {
    id: page
    signal addGameRequested

    /// Application palette passed from the main window so custom labels follow
    /// the system light/dark theme.
    property var theme

    ColumnLayout {
        anchors.fill: parent
        anchors.margins: 12
        spacing: 10

        RowLayout {
            Layout.fillWidth: true
            spacing: 8

            TabBar {
                id: tabs

                TabButton {
                    text: "GB (" + appController.library.gbCount + ")"
                }

                TabButton {
                    text: "GBC (" + appController.library.gbcCount + ")"
                }
            }

            Item {
                Layout.fillWidth: true
            }

            Button {
                text: qsTr("Add Game...")
                onClicked: page.addGameRequested()
            }

            Button {
                text: qsTr("Refresh")
                onClicked: appController.refreshLibrary()
            }
        }

        StackLayout {
            Layout.fillWidth: true
            Layout.fillHeight: true
            currentIndex: tabs.currentIndex

            Item {
                ListView {
                    id: gbList
                    anchors.fill: parent
                    clip: true
                    spacing: 2
                    model: appController.library.gbGames
                    delegate: gameDelegate
                }

                Label {
                    anchors.centerIn: parent
                    visible: gbList.count === 0
                    text: qsTr("No GB games in this library")
                    color: page.theme.windowText
                    opacity: 0.6
                }
            }

            Item {
                ListView {
                    id: gbcList
                    anchors.fill: parent
                    clip: true
                    spacing: 2
                    model: appController.library.gbcGames
                    delegate: gameDelegate
                }

                Label {
                    anchors.centerIn: parent
                    visible: gbcList.count === 0
                    text: qsTr("No GBC games in this library")
                    color: page.theme.windowText
                    opacity: 0.6
                }
            }
        }
    }

    Component {
        id: gameDelegate

        ItemDelegate {
            id: row
            width: ListView.view ? ListView.view.width : 0
            height: 66
            leftPadding: 10
            rightPadding: 6
            opacity: model.missing ? 0.45 : 1.0

            onClicked: {
                if (!model.missing) {
                    appController.openGame(model.path);
                }
            }

            contentItem: RowLayout {
                spacing: 10

                ColumnLayout {
                    Layout.fillWidth: true
                    spacing: 2

                    Label {
                        Layout.fillWidth: true
                        text: model.title
                        font.bold: true
                        color: page.theme.windowText
                        elide: Text.ElideRight
                    }

                    Label {
                        Layout.fillWidth: true
                        text: model.missing ? qsTr("File missing") : model.publisher + "  ·  " + model.region
                        color: page.theme.windowText
                        opacity: 0.65
                        font.pixelSize: 12
                        elide: Text.ElideRight
                    }
                }

                Label {
                    text: model.platform
                    color: page.theme.windowText
                    opacity: 0.55
                    font.pixelSize: 12
                }

                ToolButton {
                    text: "⋯"
                    onClicked: rowMenu.popup()

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
