import QtQuick
import QtQuick.Controls
import QtQuick.Dialogs
import QNEmu 1.0

ApplicationWindow {
    id: window

    width: 720
    height: 520
    minimumWidth: 560
    minimumHeight: 420
    visible: true
    title: qsTr("QNEmu Library")
    color: palette.window

    Theme {
        id: uiTheme
        palette: window.palette
    }

    onClosing: Qt.quit()

    menuBar: MenuBar {
        Menu {
            title: qsTr("&File")

            MenuItem {
                text: qsTr("&Open ROM...")
                action: Action {
                    text: qsTr("&Open ROM...")
                    shortcut: StandardKey.Open
                    onTriggered: openDialog.open()
                }
            }

            MenuItem {
                text: qsTr("&Add Game...")
                action: Action {
                    text: qsTr("&Add Game...")
                    onTriggered: addGameDialog.open()
                }
            }

            MenuSeparator {}

            MenuItem {
                text: qsTr("&Screenshot")
                enabled: appController.hasGame
                action: Action {
                    text: qsTr("&Screenshot")
                    shortcut: "Ctrl+Shift+S"
                    enabled: appController.hasGame
                    onTriggered: appController.takeScreenshot()
                }
            }

            MenuSeparator {}

            MenuItem {
                text: qsTr("&Settings...")
                action: Action {
                    text: qsTr("&Settings...")
                    shortcut: "Ctrl+,"
                    onTriggered: settingsDialog.showPage(settingsDialog.currentPage, window)
                }
            }

            MenuSeparator {}

            MenuItem {
                text: qsTr("E&xit")
                action: Action {
                    text: qsTr("E&xit")
                    shortcut: StandardKey.Quit
                    onTriggered: Qt.quit()
                }
            }
        }

        Menu {
            title: qsTr("&Game")

            MenuItem {
                text: appController.paused ? qsTr("&Resume") : qsTr("&Pause")
                action: Action {
                    text: appController.paused ? qsTr("&Resume") : qsTr("&Pause")
                    shortcut: "Ctrl+P"
                    enabled: appController.hasGame
                    onTriggered: {
                        if (appController.paused) {
                            appController.resume();
                            appController.showGame();
                            gameWindow.present();
                        } else {
                            appController.pause();
                        }
                    }
                }
            }
        }

        Menu {
            title: qsTr("&View")

            MenuItem {
                text: qsTr("&Library")
                enabled: appController.hasGame
                action: Action {
                    text: qsTr("&Library")
                    shortcut: "Ctrl+L"
                    enabled: appController.hasGame
                    onTriggered: {
                        appController.showLibrary();
                        window.raise();
                        window.requestActivate();
                    }
                }
            }

            MenuItem {
                text: qsTr("&Game")
                enabled: appController.hasGame
                action: Action {
                    text: qsTr("&Game")
                    shortcut: "Ctrl+G"
                    enabled: appController.hasGame
                    onTriggered: {
                        appController.showGame();
                        gameWindow.present();
                    }
                }
            }
        }
    }

    LibraryPage {
        anchors.fill: parent
        theme: uiTheme
        onAddGameRequested: addGameDialog.open()
        onSettingsRequested: function (platform) {
            settingsDialog.showPage(platform === "GBC" ? 2 : 1, window);
        }
    }

    StatusToast {
        id: statusToast
    }

    Connections {
        target: appController

        function onStatusTextChanged() {
            statusToast.show(appController.statusText);
        }

        function onShowingGameChanged() {
            if (!appController.showingGame) {
                window.raise();
                window.requestActivate();
            }
        }
    }

    FileDialog {
        id: openDialog
        title: qsTr("Open ROM")
        nameFilters: [qsTr("Game Boy ROMs (*.gb *.gbc *.sgb)"), qsTr("All files (*)")]
        onAccepted: appController.openRom(selectedFile)
    }

    FileDialog {
        id: addGameDialog
        title: qsTr("Add Game to Library")
        nameFilters: [qsTr("Game Boy ROMs (*.gb *.gbc *.sgb)"), qsTr("All files (*)")]
        onAccepted: appController.addGame(selectedFile)
    }

    SettingsDialog {
        id: settingsDialog
        parentWindow: window
    }

    GameWindow {
        id: gameWindow
        onSettingsRequested: settingsDialog.showPage(0, gameWindow)
    }
}
