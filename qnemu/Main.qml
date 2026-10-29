import QtQuick
import QtQuick.Controls
import QtQuick.Dialogs
import QNEmu 1.0

ApplicationWindow {
    id: window

    width: 640
    height: 576
    minimumWidth: 320
    minimumHeight: 288
    visible: true
    title: appController.windowTitle
    color: "black"

    menuBar: MenuBar {
        Menu {
            title: qsTr("&File")

            MenuItem {
                text: qsTr("&Open...")
                action: Action {
                    text: qsTr("&Open...")
                    shortcut: StandardKey.Open
                    onTriggered: {
                        if (!fileDialog.visible) {
                            fileDialog.open()
                        }
                    }
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
    }

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

            Component.onCompleted: {
                forceActiveFocus()
                appController.attachDisplay(emulator)
            }
        }
    }

    Label {
        anchors.centerIn: viewport
        visible: !appController.hasGame
        text: qsTr("Open a Game Boy ROM")
        color: "#e0e0e0"
        font.pixelSize: 18
    }

    FileDialog {
        id: fileDialog
        title: qsTr("Open ROM")
        nameFilters: [
            qsTr("Game Boy ROMs (*.gb *.gbc *.sgb)"),
            qsTr("All files (*)")
        ]
        onAccepted: appController.openRom(selectedFile)
    }
}
