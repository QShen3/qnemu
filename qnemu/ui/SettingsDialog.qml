import QtQuick
import QtQuick.Controls
import QtQuick.Dialogs
import QtQuick.Layouts

ApplicationWindow {
    id: settingsWindow

    property var settings: appController.settings
    property var parentWindow: null
    property int currentPage: 0
    property string capturingButton: ""

    readonly property var navItems: [
        {
            title: qsTr("General"),
            subtitle: qsTr("Display, audio and storage")
        },
        {
            title: qsTr("Game Boy"),
            subtitle: qsTr("Original Game Boy library")
        },
        {
            title: qsTr("Game Boy Color"),
            subtitle: qsTr("Game Boy Color library")
        },
        {
            title: qsTr("Controls"),
            subtitle: qsTr("Profiles and key bindings")
        }
    ]
    readonly property var launchModels: [
        {
            value: "cgb",
            label: qsTr("CGB (color)")
        },
        {
            value: "dmg",
            label: qsTr("DMG (monochrome)")
        },
        {
            value: "sgb",
            label: qsTr("SGB (DMG fallback)")
        }
    ]
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

    Theme {
        id: uiTheme
        palette: settingsWindow.palette
    }

    title: qsTr("QNEmu Settings")
    width: 760
    height: 720
    minimumWidth: 680
    minimumHeight: 560
    visible: false
    modality: Qt.ApplicationModal
    flags: Qt.Dialog
    transientParent: parentWindow

    function positionOn(anchor) {
        const target = anchor || parentWindow;
        if (!target) {
            return;
        }
        x = Math.round(target.x + (target.width - width) / 2);
        y = Math.round(target.y + (target.height - height) / 2);
    }

    function showPage(index, anchor) {
        if (index >= 0 && index < navItems.length) {
            currentPage = index;
        }
        if (!visible) {
            const target = anchor || parentWindow;
            if (target) {
                transientParent = target;
                positionOn(target);
            }
        }
        show();
        raise();
        requestActivate();
    }

    function launchModelIndex(value) {
        for (let i = 0; i < launchModels.length; ++i) {
            if (launchModels[i].value === value) {
                return i;
            }
        }
        return 0;
    }

    function buttonLabel(name) {
        for (let i = 0; i < keyActions.length; ++i) {
            if (keyActions[i].name === name) {
                return keyActions[i].label;
            }
        }
        return name;
    }

    function addProfile() {
        if (newProfileName.text.trim() === "") {
            return;
        }
        settings.addKeyProfile(newProfileName.text);
        newProfileName.text = "";
    }

    function foundText(count) {
        return count === 1 ? qsTr("1 game found") : qsTr("%1 games found").arg(count);
    }

    onClosing: capturingButton = ""

    Shortcut {
        sequence: "Esc"
        enabled: settingsWindow.capturingButton === ""
        onActivated: settingsWindow.close()
    }

    background: Rectangle {
        color: uiTheme.window
    }

    RowLayout {
        anchors.fill: parent
        spacing: 0

        Rectangle {
            Layout.preferredWidth: 200
            Layout.fillHeight: true
            color: uiTheme.sidebar

            Rectangle {
                anchors.top: parent.top
                anchors.bottom: parent.bottom
                anchors.right: parent.right
                width: 1
                color: uiTheme.separator
            }

            ColumnLayout {
                anchors.fill: parent
                anchors.margins: 16
                spacing: 3

                Label {
                    text: qsTr("Settings")
                    color: uiTheme.windowText
                    font.pixelSize: 20
                    font.weight: Font.DemiBold
                }

                Label {
                    text: qsTr("QNEmu")
                    color: uiTheme.mutedText
                    font.pixelSize: 11
                    Layout.bottomMargin: 12
                }

                Repeater {
                    model: settingsWindow.navItems

                    ItemDelegate {
                        id: navItem

                        Layout.fillWidth: true
                        height: 36
                        leftPadding: 10
                        rightPadding: 10
                        hoverEnabled: true

                        background: Rectangle {
                            radius: 7
                            color: settingsWindow.currentPage === index ? uiTheme.withAlpha(uiTheme.highlight, 0.18) : navItem.hovered ? uiTheme.hover : "transparent"
                        }

                        contentItem: RowLayout {
                            spacing: 8

                            Rectangle {
                                Layout.preferredWidth: 3
                                Layout.preferredHeight: 16
                                radius: 2
                                color: settingsWindow.currentPage === index ? uiTheme.accent : "transparent"
                            }

                            Label {
                                Layout.fillWidth: true
                                text: modelData.title
                                color: settingsWindow.currentPage === index ? uiTheme.windowText : uiTheme.mutedText
                                font.pixelSize: 12
                                font.weight: settingsWindow.currentPage === index ? Font.DemiBold : Font.Normal
                                elide: Text.ElideRight
                            }
                        }

                        onClicked: settingsWindow.currentPage = index
                    }
                }

                Item {
                    Layout.fillHeight: true
                }

                Label {
                    text: qsTr("Version %1").arg(Qt.application.version)
                    color: uiTheme.faintText
                    font.pixelSize: 11
                }
            }
        }

        ColumnLayout {
            Layout.fillWidth: true
            Layout.fillHeight: true
            Layout.margins: 24
            spacing: 14

            ColumnLayout {
                spacing: 2

                Label {
                    text: settingsWindow.navItems[settingsWindow.currentPage].title
                    color: uiTheme.windowText
                    font.pixelSize: 22
                    font.weight: Font.DemiBold
                }

                Label {
                    text: settingsWindow.navItems[settingsWindow.currentPage].subtitle
                    color: uiTheme.mutedText
                    font.pixelSize: 11
                }
            }

            StackLayout {
                Layout.fillWidth: true
                Layout.fillHeight: true
                currentIndex: settingsWindow.currentPage

                ScrollView {
                    Layout.fillWidth: true
                    Layout.fillHeight: true
                    contentWidth: availableWidth
                    clip: true

                    ColumnLayout {
                        width: parent.width
                        spacing: 14

                        SettingsCard {
                            theme: uiTheme
                            title: qsTr("Emulation")

                            RowLayout {
                                Layout.fillWidth: true
                                spacing: 16

                                ColumnLayout {
                                    Layout.fillWidth: true
                                    spacing: 2

                                    Label {
                                        text: qsTr("Launch model")
                                        color: uiTheme.windowText
                                        font.pixelSize: 12
                                    }

                                    Label {
                                        Layout.fillWidth: true
                                        text: qsTr("Used when a ROM is opened. CGB colorizes monochrome games with a compatibility palette; SGB currently falls back to DMG.")
                                        color: uiTheme.mutedText
                                        font.pixelSize: 11
                                        wrapMode: Text.WordWrap
                                    }
                                }

                                ComboBox {
                                    Layout.preferredWidth: 180
                                    model: settingsWindow.launchModels
                                    textRole: "label"
                                    currentIndex: settingsWindow.launchModelIndex(settings.launchModel)
                                    onActivated: settings.launchModel = settingsWindow.launchModels[currentIndex].value
                                }
                            }
                        }

                        SettingsCard {
                            theme: uiTheme
                            title: qsTr("Display")

                            RowLayout {
                                Layout.fillWidth: true
                                spacing: 16

                                ColumnLayout {
                                    Layout.fillWidth: true
                                    spacing: 2

                                    Label {
                                        text: qsTr("Game window scale")
                                        color: uiTheme.windowText
                                        font.pixelSize: 12
                                    }

                                    Label {
                                        Layout.fillWidth: true
                                        text: qsTr("Only the game window: 100% is 320×288; 200% is 640×576. The library window is independent.")
                                        color: uiTheme.mutedText
                                        font.pixelSize: 11
                                        wrapMode: Text.WordWrap
                                    }
                                }

                                ComboBox {
                                    Layout.preferredWidth: 130
                                    model: ["100%", "150%", "200%"]
                                    currentIndex: settings.scalePercent === 100 ? 0 : settings.scalePercent === 150 ? 1 : 2
                                    onActivated: settings.scalePercent = [100, 150, 200][currentIndex]
                                }
                            }

                            Rectangle {
                                Layout.fillWidth: true
                                height: 1
                                color: uiTheme.separator
                            }

                            RowLayout {
                                Layout.fillWidth: true
                                spacing: 16

                                ColumnLayout {
                                    Layout.fillWidth: true
                                    spacing: 2

                                    Label {
                                        text: qsTr("Play sound")
                                        color: uiTheme.windowText
                                        font.pixelSize: 12
                                    }

                                    Label {
                                        Layout.fillWidth: true
                                        text: qsTr("Turn off to mute the emulator's audio output.")
                                        color: uiTheme.mutedText
                                        font.pixelSize: 11
                                        wrapMode: Text.WordWrap
                                    }
                                }

                                Switch {
                                    checked: settings.soundEnabled
                                    onToggled: settings.soundEnabled = checked
                                }
                            }
                        }

                        SettingsCard {
                            theme: uiTheme
                            title: qsTr("Screenshots")

                            RowLayout {
                                Layout.fillWidth: true
                                spacing: 10

                                ColumnLayout {
                                    Layout.fillWidth: true
                                    spacing: 2

                                    Label {
                                        text: qsTr("Folder")
                                        color: uiTheme.windowText
                                        font.pixelSize: 12
                                    }

                                    Label {
                                        Layout.fillWidth: true
                                        text: qsTr("Saved as ROM name plus date.")
                                        color: uiTheme.mutedText
                                        font.pixelSize: 11
                                        wrapMode: Text.WordWrap
                                    }

                                    RowLayout {
                                        Layout.fillWidth: true
                                        Layout.topMargin: 4
                                        spacing: 8

                                        TextField {
                                            Layout.fillWidth: true
                                            text: settings.screenshotDirectory
                                            onEditingFinished: settings.screenshotDirectory = text
                                        }

                                        Button {
                                            text: qsTr("Browse…")
                                            onClicked: {
                                                folderDialog.target = "screenshots";
                                                folderDialog.open();
                                            }
                                        }
                                    }
                                }
                            }

                            RowLayout {
                                Layout.fillWidth: true
                                spacing: 10

                                ColumnLayout {
                                    Layout.preferredWidth: 120
                                    spacing: 2

                                    Label {
                                        text: qsTr("Format")
                                        color: uiTheme.windowText
                                        font.pixelSize: 12
                                    }

                                    Label {
                                        Layout.fillWidth: true
                                        text: qsTr("Formats provided by Qt.")
                                        color: uiTheme.mutedText
                                        font.pixelSize: 11
                                        wrapMode: Text.WordWrap
                                    }
                                }

                                ComboBox {
                                    Layout.preferredWidth: 130
                                    model: settings.screenshotFormats
                                    currentIndex: Math.max(0, settings.screenshotFormats.indexOf(settings.screenshotFormat))
                                    onActivated: settings.screenshotFormat = settings.screenshotFormats[currentIndex]
                                }

                                Item {
                                    Layout.fillWidth: true
                                }
                            }
                        }

                        SettingsCard {
                            theme: uiTheme
                            title: qsTr("Storage")

                            RowLayout {
                                Layout.fillWidth: true
                                spacing: 10

                                ColumnLayout {
                                    Layout.fillWidth: true
                                    spacing: 2

                                    Label {
                                        text: qsTr("Save folder")
                                        color: uiTheme.windowText
                                        font.pixelSize: 12
                                    }

                                    Label {
                                        Layout.fillWidth: true
                                        text: qsTr("Battery saves and RTC data.")
                                        color: uiTheme.mutedText
                                        font.pixelSize: 11
                                        wrapMode: Text.WordWrap
                                    }

                                    RowLayout {
                                        Layout.fillWidth: true
                                        Layout.topMargin: 4
                                        spacing: 8

                                        TextField {
                                            Layout.fillWidth: true
                                            text: settings.savePath
                                            onEditingFinished: settings.savePath = text
                                        }

                                        Button {
                                            text: qsTr("Browse…")
                                            onClicked: {
                                                folderDialog.target = "saves";
                                                folderDialog.open();
                                            }
                                        }
                                    }
                                }
                            }
                        }

                        Item {
                            Layout.fillHeight: true
                        }
                    }
                }

                ScrollView {
                    Layout.fillWidth: true
                    Layout.fillHeight: true
                    contentWidth: availableWidth
                    clip: true

                    ColumnLayout {
                        width: parent.width
                        spacing: 14

                        SettingsCard {
                            theme: uiTheme
                            title: qsTr("Library folder")

                            Label {
                                Layout.fillWidth: true
                                text: qsTr("Original Game Boy ROMs are scanned recursively from this folder.")
                                color: uiTheme.mutedText
                                font.pixelSize: 11
                                wrapMode: Text.WordWrap
                            }

                            RowLayout {
                                Layout.fillWidth: true
                                spacing: 10

                                TextField {
                                    Layout.fillWidth: true
                                    text: settings.gbLibraryPath
                                    onEditingFinished: settings.gbLibraryPath = text
                                }

                                Button {
                                    text: qsTr("Browse…")
                                    onClicked: {
                                        folderDialog.target = "gb";
                                        folderDialog.open();
                                    }
                                }
                            }

                            RowLayout {
                                Layout.fillWidth: true

                                Label {
                                    text: settingsWindow.foundText(appController.library.gbCount)
                                    color: uiTheme.mutedText
                                    font.pixelSize: 11
                                }

                                Item {
                                    Layout.fillWidth: true
                                }

                                Button {
                                    text: qsTr("Rescan")
                                    onClicked: appController.refreshLibrary()
                                }
                            }
                        }

                        Item {
                            Layout.fillHeight: true
                        }
                    }
                }

                ScrollView {
                    Layout.fillWidth: true
                    Layout.fillHeight: true
                    contentWidth: availableWidth
                    clip: true

                    ColumnLayout {
                        width: parent.width
                        spacing: 14

                        SettingsCard {
                            theme: uiTheme
                            title: qsTr("Library folder")

                            Label {
                                Layout.fillWidth: true
                                text: qsTr("Game Boy Color ROMs are scanned recursively from this folder.")
                                color: uiTheme.mutedText
                                font.pixelSize: 11
                                wrapMode: Text.WordWrap
                            }

                            RowLayout {
                                Layout.fillWidth: true
                                spacing: 10

                                TextField {
                                    Layout.fillWidth: true
                                    text: settings.gbcLibraryPath
                                    onEditingFinished: settings.gbcLibraryPath = text
                                }

                                Button {
                                    text: qsTr("Browse…")
                                    onClicked: {
                                        folderDialog.target = "gbc";
                                        folderDialog.open();
                                    }
                                }
                            }

                            RowLayout {
                                Layout.fillWidth: true

                                Label {
                                    text: settingsWindow.foundText(appController.library.gbcCount)
                                    color: uiTheme.mutedText
                                    font.pixelSize: 11
                                }

                                Item {
                                    Layout.fillWidth: true
                                }

                                Button {
                                    text: qsTr("Rescan")
                                    onClicked: appController.refreshLibrary()
                                }
                            }
                        }

                        Item {
                            Layout.fillHeight: true
                        }
                    }
                }

                ScrollView {
                    Layout.fillWidth: true
                    Layout.fillHeight: true
                    contentWidth: availableWidth
                    clip: true

                    ColumnLayout {
                        width: parent.width
                        spacing: 14

                        SettingsCard {
                            theme: uiTheme
                            title: qsTr("Profile")

                            RowLayout {
                                Layout.fillWidth: true
                                spacing: 10

                                Label {
                                    Layout.preferredWidth: 100
                                    text: qsTr("Active profile")
                                    color: uiTheme.windowText
                                    font.pixelSize: 12
                                }

                                ComboBox {
                                    Layout.fillWidth: true
                                    model: settings.profileNames
                                    currentIndex: Math.max(0, settings.profileNames.indexOf(settings.activeKeyProfile))
                                    onActivated: settings.activeKeyProfile = settings.profileNames[currentIndex]
                                }

                                Button {
                                    text: qsTr("Delete")
                                    enabled: settings.profileNames.length > 1
                                    onClicked: settings.removeKeyProfile(settings.activeKeyProfile)
                                }
                            }

                            RowLayout {
                                Layout.fillWidth: true
                                spacing: 10

                                TextField {
                                    id: newProfileName
                                    Layout.fillWidth: true
                                    placeholderText: qsTr("New profile name")
                                    onAccepted: settingsWindow.addProfile()
                                }

                                Button {
                                    text: qsTr("Add profile")
                                    onClicked: settingsWindow.addProfile()
                                }
                            }
                        }

                        SettingsCard {
                            theme: uiTheme
                            title: qsTr("Key bindings")

                            Label {
                                Layout.fillWidth: true
                                text: qsTr("Click a key button, then press the new key. Esc cancels.")
                                color: uiTheme.mutedText
                                font.pixelSize: 11
                                wrapMode: Text.WordWrap
                            }

                            GridLayout {
                                Layout.fillWidth: true
                                columns: 2
                                columnSpacing: 16
                                rowSpacing: 8

                                Repeater {
                                    model: settingsWindow.keyActions

                                    RowLayout {
                                        Layout.fillWidth: true
                                        spacing: 8

                                        Label {
                                            Layout.preferredWidth: 56
                                            text: modelData.label
                                            color: uiTheme.mutedText
                                            font.pixelSize: 11
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

                        Item {
                            Layout.fillHeight: true
                        }
                    }
                }
            }

            RowLayout {
                Layout.fillWidth: true

                Label {
                    text: qsTr("Changes are applied immediately.")
                    color: uiTheme.faintText
                    font.pixelSize: 11
                }

                Item {
                    Layout.fillWidth: true
                }

                Button {
                    text: qsTr("Close")
                    onClicked: settingsWindow.close()
                }
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
            color: "#66000000"
        }

        Rectangle {
            anchors.centerIn: parent
            width: 360
            height: 150
            radius: 12
            color: uiTheme.card
            border.width: 1
            border.color: uiTheme.border

            ColumnLayout {
                anchors.centerIn: parent
                spacing: 6

                Label {
                    Layout.alignment: Qt.AlignHCenter
                    text: qsTr("Press a key for")
                    color: uiTheme.mutedText
                    font.pixelSize: 11
                }

                Label {
                    Layout.alignment: Qt.AlignHCenter
                    text: settingsWindow.buttonLabel(settingsWindow.capturingButton)
                    color: uiTheme.windowText
                    font.pixelSize: 20
                    font.weight: Font.DemiBold
                }

                Label {
                    Layout.alignment: Qt.AlignHCenter
                    text: qsTr("Esc to cancel")
                    color: uiTheme.faintText
                    font.pixelSize: 11
                }
            }
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
