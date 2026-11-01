import QtQuick
import QtQuick.Controls
import QNEmu 1.0

ApplicationWindow {
    id: linkWindow

    visible: false
    title: appController.linkInfrared
        ? qsTr("QNEmu Infrared Link (%1 players)")
            .arg(appController.linkPlayerCount)
        : qsTr("QNEmu Link Play (%1 players)")
            .arg(appController.linkPlayerCount)
    color: "black"

    readonly property int playerCount: appController.linkPlayerCount
    readonly property int rows: playerCount > 2 ? 2 : 1
    readonly property real effectiveScale: playerCount > 2
        ? Math.min(appController.scaleFactor, 1.0)
        : appController.scaleFactor
    readonly property real paneHeight: Math.round(144 * 2 * effectiveScale)
    readonly property real paneWidth: Math.round(paneHeight * 160 / 144)

    width: paneWidth * 2 + 3
    height: paneHeight * rows + (rows - 1) * 2 + (menuBar ? menuBar.height : 0)
    minimumWidth: 320
    minimumHeight: 160

    function present() {
        show();
        raise();
        requestActivate();
        Qt.callLater(function () {
            const firstPane = paneRepeater.itemAt(0);
            if (firstPane) {
                firstPane.display.forceActiveFocus();
            }
        });
    }

    function releaseAllButtons() {
        for (let index = 0; index < paneRepeater.count; ++index) {
            const pane = paneRepeater.itemAt(index);
            if (pane && pane.display) {
                pane.display.releaseAllButtons();
            }
        }
    }

    function titleAt(index) {
        switch (index) {
        case 0:
            return appController.linkTitle1;
        case 1:
            return appController.linkTitle2;
        case 2:
            return appController.linkTitle3;
        default:
            return appController.linkTitle4;
        }
    }

    Component.onCompleted: {
        if (appController.linkActive) {
            present();
        }
    }

    onClosing: {
        releaseAllButtons();
        appController.stopLink();
    }

    onActiveChanged: {
        if (active) {
            const firstPane = paneRepeater.itemAt(0);
            if (firstPane) {
                firstPane.display.forceActiveFocus();
            }
        } else {
            releaseAllButtons();
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

    Grid {
        anchors.fill: parent
        columns: 2
        spacing: 2

        Repeater {
            id: paneRepeater

            model: 4

            delegate: Rectangle {
                id: pane

                required property int index

                property alias display: emulator

                visible: index < linkWindow.playerCount
                width: linkWindow.paneWidth
                height: linkWindow.paneHeight
                color: "black"
                border.width: emulator.activeFocus ? 2 : 0
                border.color: "#4a9eff"

                EmulatorItem {
                    id: emulator

                    anchors.fill: parent
                    focus: index === 0
                }

                TapHandler {
                    onTapped: emulator.forceActiveFocus()
                }

                Rectangle {
                    anchors.left: parent.left
                    anchors.top: parent.top
                    anchors.margins: 6
                    width: titleLabel.implicitWidth + 12
                    height: titleLabel.implicitHeight + 6
                    radius: 4
                    color: "#a0000000"

                    Label {
                        id: titleLabel

                        anchors.centerIn: parent
                        color: "white"
                        font.pixelSize: 11
                        text: qsTr("P%1 · %2")
                            .arg(pane.index + 1)
                            .arg(linkWindow.titleAt(pane.index))
                    }
                }

                Component.onCompleted: {
                    appController.attachLinkDisplay(pane.index, emulator);
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
