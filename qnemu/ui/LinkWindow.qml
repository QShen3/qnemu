import QtQuick
import QtQuick.Controls
import QNEmu 1.0

ApplicationWindow {
    id: linkWindow

    visible: false
    title: qsTr("QNEmu Link Play")
    color: "black"

    readonly property real paneHeight: Math.round(144 * 2 * appController.scaleFactor)
    readonly property real paneWidth1: Math.round(paneHeight * emulator1.frameWidth / Math.max(1, emulator1.frameHeight))
    readonly property real paneWidth2: Math.round(paneHeight * emulator2.frameWidth / Math.max(1, emulator2.frameHeight))

    width: paneWidth1 + paneWidth2 + 3
    height: paneHeight + (menuBar ? menuBar.height : 0)
    minimumWidth: 320
    minimumHeight: 160

    function present() {
        show();
        raise();
        requestActivate();
        Qt.callLater(function () {
            emulator1.forceActiveFocus();
        });
    }

    Component.onCompleted: {
        appController.attachLinkDisplay(0, emulator1);
        appController.attachLinkDisplay(1, emulator2);
        if (appController.linkActive) {
            present();
        }
    }

    onClosing: {
        emulator1.releaseAllButtons();
        emulator2.releaseAllButtons();
        appController.stopLink();
    }

    onActiveChanged: {
        if (active) {
            emulator1.forceActiveFocus();
        } else {
            emulator1.releaseAllButtons();
            emulator2.releaseAllButtons();
        }
    }

    Connections {
        target: appController

        function onLinkActiveChanged() {
            if (appController.linkActive) {
                linkWindow.present();
            } else {
                linkWindow.hide();
            }
        }

        function onStatusTextChanged() {
            toast.show(appController.statusText);
        }
    }

    menuBar: MenuBar {
        Menu {
            title: qsTr("&Game")

            MenuItem {
                text: appController.linkPaused ? qsTr("&Resume") : qsTr("&Pause")
                action: Action {
                    text: appController.linkPaused ? qsTr("&Resume") : qsTr("&Pause")
                    shortcut: "Space"
                    onTriggered: appController.toggleLinkPause()
                }
            }

            MenuSeparator {}

            MenuItem {
                text: qsTr("&Back to Library")
                action: Action {
                    text: qsTr("&Back to Library")
                    shortcut: "Ctrl+L"
                    onTriggered: {
                        linkWindow.close();
                        appController.showLibrary();
                    }
                }
            }

            MenuSeparator {}

            MenuItem {
                text: qsTr("&Close")
                action: Action {
                    text: qsTr("&Close")
                    shortcut: StandardKey.Close
                    onTriggered: linkWindow.close()
                }
            }
        }
    }

    Shortcut {
        sequence: "Ctrl+P"
        enabled: appController.linkActive
        onActivated: appController.toggleLinkPause()
    }

    Row {
        anchors.fill: parent
        spacing: 2

        Rectangle {
            id: pane1

            width: linkWindow.paneWidth1
            height: linkWindow.paneHeight
            color: "black"
            border.width: emulator1.activeFocus ? 2 : 0
            border.color: "#4a9eff"

            EmulatorItem {
                id: emulator1

                anchors.fill: parent
                focus: true
            }

            TapHandler {
                onTapped: emulator1.forceActiveFocus()
            }

            Rectangle {
                anchors.left: parent.left
                anchors.top: parent.top
                anchors.margins: 6
                width: label1.implicitWidth + 12
                height: label1.implicitHeight + 6
                radius: 4
                color: "#a0000000"

                Label {
                    id: label1

                    anchors.centerIn: parent
                    color: "white"
                    font.pixelSize: 11
                    text: qsTr("P1 · ") + appController.linkTitle1
                }
            }
        }

        Rectangle {
            width: 1
            height: parent.height
            color: "#333333"
        }

        Rectangle {
            id: pane2

            width: linkWindow.paneWidth2
            height: linkWindow.paneHeight
            color: "black"
            border.width: emulator2.activeFocus ? 2 : 0
            border.color: "#4a9eff"

            EmulatorItem {
                id: emulator2

                anchors.fill: parent
            }

            TapHandler {
                onTapped: emulator2.forceActiveFocus()
            }

            Rectangle {
                anchors.left: parent.left
                anchors.top: parent.top
                anchors.margins: 6
                width: label2.implicitWidth + 12
                height: label2.implicitHeight + 6
                radius: 4
                color: "#a0000000"

                Label {
                    id: label2

                    anchors.centerIn: parent
                    color: "white"
                    font.pixelSize: 11
                    text: qsTr("P2 · ") + appController.linkTitle2
                }
            }
        }
    }

    Rectangle {
        id: pauseOverlay

        anchors.centerIn: parent
        width: 260
        height: 112
        radius: 12
        color: "#d9000000"
        border.width: 1
        border.color: "#33ffffff"
        visible: appController.linkPaused
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
