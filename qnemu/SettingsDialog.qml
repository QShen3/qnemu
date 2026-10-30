import QtQuick
import QtQuick.Controls
import QtQuick.Dialogs
import QtQuick.Layouts

ApplicationWindow {
    id: settingsWindow

    property var settings: appController.settings
    property var parentWindow: null
    property string capturingButton: ""
    readonly property var keyActions: [
        {
            name: "up",
            label: qsTr("Up")
        },
        {
            name: "down",
            label: qsTr("Down")
        },
        {
            name: "left",
            label: qsTr("Left")
        },
        {
            name: "right",
            label: qsTr("Right")
        },
        {
            name: "a",
            label: qsTr("A")
        },
        {
            name: "b",
            label: qsTr("B")
        },
        {
            name: "select",
            label: qsTr("Select")
        },
        {
            name: "start",
            label: qsTr("Start")
        }
    ]

    title: qsTr("QNEmu Settings")
    width: 720
    height: 560
    visible: false
    modality: Qt.ApplicationModal
    flags: Qt.Dialog
    transientParent: parentWindow
    x: parentWindow ? parentWindow.x + (parentWindow.width - width) / 2 : 0
    y: parentWindow ? parentWindow.y + (parentWindow.height - height) / 2 : 0

    Shortcut {
        sequence: "Esc"
        enabled: settingsWindow.capturingButton === ""
        onActivated: settingsWindow.close()
    }

    ColumnLayout {
        anchors.fill: parent
        anchors.margins: 12
        spacing: 8

        TabBar {
            id: tabs
            Layout.fillWidth: true

            TabButton {
                text: qsTr("General")
            }
            TabButton {
                text: qsTr("GB")
            }
            TabButton {
                text: qsTr("GBC")
            }
            TabButton {
                text: qsTr("Controls")
            }
        }

        StackLayout {
            Layout.fillWidth: true
            Layout.fillHeight: true
            currentIndex: tabs.currentIndex

            ScrollView {
                contentWidth: availableWidth
                Layout.fillWidth: true
                Layout.fillHeight: true

                ColumnLayout {
                    width: parent.width
                    spacing: 10

                    Label {
                        text: qsTr("Display")
                        font.bold: true
                    }

                    RowLayout {
                        Layout.fillWidth: true

                        Label {
                            text: qsTr("Scale")
                            Layout.preferredWidth: 150
                        }

                        ComboBox {
                            model: ["100%", "150%", "200%"]
                            currentIndex: settings.scalePercent === 100 ? 0 : settings.scalePercent === 150 ? 1 : 2
                            onActivated: settings.scalePercent = [100, 150, 200][currentIndex]
                        }

                        Item {
                            Layout.fillWidth: true
                        }
                    }

                    Switch {
                        text: qsTr("Play sound")
                        checked: settings.soundEnabled
                        onToggled: settings.soundEnabled = checked
                    }

                    Label {
                        text: qsTr("Screenshots")
                        font.bold: true
                    }

                    Label {
                        text: qsTr("Output folder")
                    }
                    RowLayout {
                        Layout.fillWidth: true

                        TextField {
                            Layout.fillWidth: true
                            text: settings.screenshotDirectory
                            onEditingFinished: settings.screenshotDirectory = text
                        }

                        Button {
                            text: qsTr("Browse...")
                            onClicked: {
                                folderDialog.target = "screenshots";
                                folderDialog.open();
                            }
                        }
                    }

                    RowLayout {
                        Layout.fillWidth: true

                        Label {
                            text: qsTr("Format")
                            Layout.preferredWidth: 150
                        }

                        ComboBox {
                            model: settings.screenshotFormats
                            currentIndex: Math.max(0, settings.screenshotFormats.indexOf(settings.screenshotFormat))
                            onActivated: settings.screenshotFormat = settings.screenshotFormats[currentIndex]
                        }

                        Item {
                            Layout.fillWidth: true
                        }
                    }

                    Label {
                        text: qsTr("Saves")
                        font.bold: true
                    }

                    Label {
                        text: qsTr("Save folder")
                    }
                    RowLayout {
                        Layout.fillWidth: true

                        TextField {
                            Layout.fillWidth: true
                            text: settings.savePath
                            onEditingFinished: settings.savePath = text
                        }

                        Button {
                            text: qsTr("Browse...")
                            onClicked: {
                                folderDialog.target = "saves";
                                folderDialog.open();
                            }
                        }
                    }

                    Item {
                        Layout.fillHeight: true
                    }
                }
            }

            ScrollView {
                contentWidth: availableWidth
                Layout.fillWidth: true
                Layout.fillHeight: true

                ColumnLayout {
                    width: parent.width
                    spacing: 10

                    Label {
                        text: qsTr("Game Boy library")
                        font.bold: true
                    }

                    Label {
                        text: qsTr("ROM folder for original Game Boy games")
                    }
                    RowLayout {
                        Layout.fillWidth: true

                        TextField {
                            Layout.fillWidth: true
                            text: settings.gbLibraryPath
                            onEditingFinished: settings.gbLibraryPath = text
                        }

                        Button {
                            text: qsTr("Browse...")
                            onClicked: {
                                folderDialog.target = "gb";
                                folderDialog.open();
                            }
                        }
                    }
                }
            }

            ScrollView {
                contentWidth: availableWidth
                Layout.fillWidth: true
                Layout.fillHeight: true

                ColumnLayout {
                    width: parent.width
                    spacing: 10

                    Label {
                        text: qsTr("Game Boy Color library")
                        font.bold: true
                    }

                    Label {
                        text: qsTr("ROM folder for Game Boy Color games")
                    }
                    RowLayout {
                        Layout.fillWidth: true

                        TextField {
                            Layout.fillWidth: true
                            text: settings.gbcLibraryPath
                            onEditingFinished: settings.gbcLibraryPath = text
                        }

                        Button {
                            text: qsTr("Browse...")
                            onClicked: {
                                folderDialog.target = "gbc";
                                folderDialog.open();
                            }
                        }
                    }
                }
            }

            ScrollView {
                contentWidth: availableWidth
                Layout.fillWidth: true
                Layout.fillHeight: true

                ColumnLayout {
                    width: parent.width
                    spacing: 10

                    Label {
                        text: qsTr("Controller")
                        font.bold: true
                    }

                    RowLayout {
                        Layout.fillWidth: true

                        Label {
                            text: qsTr("Profile")
                            Layout.preferredWidth: 150
                        }

                        ComboBox {
                            Layout.fillWidth: true
                            model: settings.profileNames
                            currentIndex: Math.max(0, settings.profileNames.indexOf(settings.activeKeyProfile))
                            onActivated: settings.activeKeyProfile = settings.profileNames[currentIndex]
                        }

                        Button {
                            text: qsTr("Delete")
                            onClicked: settings.removeKeyProfile(settings.activeKeyProfile)
                        }
                    }

                    RowLayout {
                        Layout.fillWidth: true

                        TextField {
                            id: newProfileName
                            Layout.fillWidth: true
                            placeholderText: qsTr("New profile name")
                        }

                        Button {
                            text: qsTr("Add profile")
                            onClicked: {
                                settings.addKeyProfile(newProfileName.text);
                                newProfileName.text = "";
                            }
                        }
                    }

                    Label {
                        text: qsTr("Click a key button, then press the new key.")
                        opacity: 0.65
                    }

                    Repeater {
                        model: settingsWindow.keyActions

                        RowLayout {
                            Layout.fillWidth: true

                            Label {
                                text: modelData.label
                                Layout.preferredWidth: 100
                            }

                            Button {
                                Layout.fillWidth: true
                                text: settings.keyName(settings.activeKeyBindings[modelData.name] || 0)
                                onClicked: {
                                    settingsWindow.capturingButton = modelData.name;
                                    captureOverlay.forceActiveFocus();
                                }
                            }
                        }
                    }
                }
            }
        }

        RowLayout {
            Layout.fillWidth: true

            Item {
                Layout.fillWidth: true
            }

            Button {
                text: qsTr("Close")
                onClicked: settingsWindow.close()
            }
        }
    }

    FolderDialog {
        id: folderDialog
        property string target
        title: qsTr("Choose folder")

        onAccepted: {
            const path = appController.localPath(selectedFolder);
            if (target === "gb") {
                settings.gbLibraryPath = path;
            } else if (target === "gbc") {
                settings.gbcLibraryPath = path;
            } else if (target === "saves") {
                settings.savePath = path;
            } else if (target === "screenshots") {
                settings.screenshotDirectory = path;
            }
        }
    }

    Item {
        id: captureOverlay
        anchors.fill: parent
        visible: settingsWindow.capturingButton !== ""
        focus: visible
        z: 100

        Rectangle {
            anchors.fill: parent
            color: "#99000000"
        }

        Label {
            anchors.centerIn: parent
            color: "white"
            font.pixelSize: 20
            text: qsTr("Press a key for ") + settingsWindow.capturingButton
        }

        Keys.onPressed: function (event) {
            if (event.key === Qt.Key_Escape) {
                settingsWindow.capturingButton = "";
            } else {
                settings.setKeyBinding(settings.activeKeyProfile, settingsWindow.capturingButton, event.key);
                settingsWindow.capturingButton = "";
            }
            event.accepted = true;
        }

        onVisibleChanged: {
            if (visible) {
                forceActiveFocus();
            }
        }
    }
}
