import QtQuick
import QtQuick.Controls
import QtQuick.Dialogs
import QtQuick.Layouts
import QNEmu 1.0

ApplicationWindow {
    id: window

    width: 320
    height: 288
    minimumWidth: 240
    minimumHeight: 220
    visible: true
    title: appController.windowTitle
    color: "#1b1b1b"

    function applyScale() {
        const factor = appController.scaleFactor;
        width = Math.round(320 * factor);
        height = Math.round(288 * factor) + (menuBar ? menuBar.height : 0);
    }

    Component.onCompleted: {
        appController.attachDisplay(emulator);
        Qt.callLater(function () {
            window.applyScale();
            if (appController.showingGame) {
                emulator.forceActiveFocus();
            }
        });
    }

    onActiveChanged: {
        if (active && appController.showingGame) {
            emulator.forceActiveFocus();
        }
    }

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
                    onTriggered: settingsDialog.show()
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
            title: qsTr("&View")

            MenuItem {
                text: qsTr("&Library")
                enabled: appController.hasGame
                action: Action {
                    text: qsTr("&Library")
                    shortcut: "Ctrl+L"
                    enabled: appController.hasGame
                    onTriggered: appController.showLibrary()
                }
            }

            MenuItem {
                text: qsTr("&Game")
                enabled: appController.hasGame
                action: Action {
                    text: qsTr("&Game")
                    shortcut: "Ctrl+G"
                    enabled: appController.hasGame
                    onTriggered: appController.showGame()
                }
            }
        }
    }

    StackLayout {
        anchors.fill: parent
        currentIndex: appController.showingGame ? 1 : 0

        LibraryPage {
            id: libraryPage
            onAddGameRequested: addGameDialog.open()
        }

        Item {
            id: gamePage

            Item {
                id: viewport
                anchors.centerIn: parent
                width: Math.min(parent.width, parent.height * 160 / 144)
                height: width * 144 / 160

                EmulatorItem {
                    id: emulator
                    anchors.fill: parent
                    focus: true

                    TapHandler {
                        onTapped: emulator.forceActiveFocus()
                    }
                }
            }
        }
    }

    Rectangle {
        id: statusToast
        visible: statusLabel.text.length > 0
        anchors.horizontalCenter: parent.horizontalCenter
        anchors.bottom: parent.bottom
        anchors.bottomMargin: 12
        width: Math.min(parent.width - 24, statusLabel.implicitWidth + 24)
        height: statusLabel.implicitHeight + 12
        radius: 6
        color: "#cc000000"

        Label {
            id: statusLabel
            anchors.centerIn: parent
            width: parent.width - 24
            horizontalAlignment: Text.AlignHCenter
            wrapMode: Text.Wrap
            color: "white"
        }
    }

    Timer {
        id: statusTimer
        interval: 4000
        onTriggered: statusLabel.text = ""
    }

    Connections {
        target: appController

        function onStatusTextChanged() {
            statusLabel.text = appController.statusText;
            statusTimer.restart();
        }

        function onScaleFactorChanged() {
            window.applyScale();
        }

        function onShowingGameChanged() {
            if (appController.showingGame) {
                Qt.callLater(function () {
                    emulator.forceActiveFocus();
                });
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
}
