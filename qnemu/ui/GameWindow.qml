import QtQuick
import QtQuick.Controls
import QNEmu 1.0

ApplicationWindow {
    id: gameWindow

    signal settingsRequested

    width: 320
    height: 288
    minimumWidth: 160
    minimumHeight: 144
    visible: false
    title: appController.windowTitle + (appController.paused ? qsTr(" (Paused)") : "")
    color: "black"

    function applyScale() {
        const factor = appController.scaleFactor;
        width = Math.round(320 * factor);
        height = Math.round(288 * factor) + (menuBar ? menuBar.height : 0);
    }

    function present() {
        applyScale();
        show();
        raise();
        requestActivate();
        Qt.callLater(function () {
            emulator.forceActiveFocus();
        });
    }

    Component.onCompleted: {
        appController.attachDisplay(emulator);
        if (appController.showingGame) {
            present();
        }
    }

    onActiveChanged: {
        if (active) {
            emulator.forceActiveFocus();
        } else {
            emulator.releaseAllButtons();
        }
    }

    onClosing: {
        emulator.releaseAllButtons();
        appController.showLibrary();
    }

    Connections {
        target: appController

        function onShowingGameChanged() {
            if (appController.showingGame) {
                gameWindow.present();
            } else {
                gameWindow.hide();
            }
        }

        function onScaleFactorChanged() {
            gameWindow.applyScale();
        }

        function onStatusTextChanged() {
            toast.show(appController.statusText);
        }
    }

    menuBar: MenuBar {
        Menu {
            title: qsTr("&Game")

            MenuItem {
                text: appController.paused ? qsTr("&Resume") : qsTr("&Pause")
                action: Action {
                    text: appController.paused ? qsTr("&Resume") : qsTr("&Pause")
                    shortcut: "Space"
                    enabled: appController.hasGame
                    onTriggered: appController.togglePause()
                }
            }

            MenuSeparator {}

            MenuItem {
                text: qsTr("&Back to Library")
                action: Action {
                    text: qsTr("&Back to Library")
                    shortcut: "Ctrl+L"
                    onTriggered: appController.showLibrary()
                }
            }

            MenuItem {
                text: qsTr("&Screenshot")
                action: Action {
                    text: qsTr("&Screenshot")
                    shortcut: "Ctrl+Shift+S"
                    onTriggered: appController.takeScreenshot()
                }
            }

            MenuSeparator {}

            MenuItem {
                text: qsTr("S&ettings...")
                action: Action {
                    text: qsTr("S&ettings...")
                    shortcut: "Ctrl+,"
                    onTriggered: gameWindow.settingsRequested()
                }
            }

            MenuSeparator {}

            MenuItem {
                text: qsTr("&Close")
                action: Action {
                    text: qsTr("&Close")
                    shortcut: StandardKey.Close
                    onTriggered: gameWindow.close()
                }
            }
        }

        Menu {
            title: qsTr("&View")

            MenuItem {
                text: qsTr("&Library")
                action: Action {
                    text: qsTr("&Library")
                    shortcut: "Ctrl+L"
                    onTriggered: appController.showLibrary()
                }
            }
        }
    }

    Shortcut {
        sequence: "Ctrl+P"
        enabled: appController.hasGame
        onActivated: appController.togglePause()
    }

    Rectangle {
        anchors.fill: parent
        color: "black"

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

    Rectangle {
        id: pauseOverlay

        anchors.centerIn: parent
        width: 240
        height: 112
        radius: 12
        color: "#d9000000"
        border.width: 1
        border.color: "#33ffffff"
        visible: appController.paused
        z: 40

        Column {
            anchors.centerIn: parent
            spacing: 4

            Label {
                anchors.horizontalCenter: parent.horizontalCenter
                text: qsTr("Paused")
                color: "white"
                font.pixelSize: 22
                font.weight: Font.DemiBold
            }

            Label {
                anchors.horizontalCenter: parent.horizontalCenter
                text: qsTr("Space or Ctrl+P to resume")
                color: "#b3ffffff"
                font.pixelSize: 11
            }
        }
    }

    StatusToast {
        id: toast
    }
}
