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
        if (appController.library.loading) {
            return qsTr("Scanning game library...");
        }
        const count = appController.library.gbCount + appController.library.gbcCount;
        if (appController.library.filterText.trim() !== "") {
            return count === 1 ? qsTr("1 matching game") : qsTr("%1 matching games").arg(count);
        }
        if (count === 0) {
            return qsTr("No games in your library");
        }
        return count === 1 ? qsTr("1 game in your library") : qsTr("%1 games in your library").arg(count);
    }

    function statusLabel(status) {
        switch (status) {
        case "official":
            return qsTr("Official");
        case "aftermarket":
            return qsTr("Aftermarket");
        case "unlicensed":
            return qsTr("Unlicensed");
        case "proto":
            return qsTr("Prototype");
        case "pirate":
            return qsTr("Pirate");
        case "homebrew":
            return qsTr("Homebrew");
        case "demo":
            return qsTr("Demo");
        default:
            return "";
        }
    }

    function statusColor(status) {
        if (status === "official") {
            return page.theme.accent;
        }
        if (status === "aftermarket" || status === "demo") {
            return page.theme.withAlpha(page.theme.windowText, 0.14);
        }
        return page.theme.warning;
    }

    function statusTextColor(status) {
        if (status === "official") {
            return page.theme.accentText;
        }
        if (status === "aftermarket" || status === "demo") {
            return page.theme.windowText;
        }
        return "#1b1b1b";
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
                enabled: !appController.library.loading
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
                enabled: !appController.library.loading
                ToolTip.visible: hovered
                ToolTip.text: qsTr("Rescan library folders")
                onClicked: appController.refreshLibrary()
            }
        }

        RowLayout {
            Layout.fillWidth: true
            spacing: 4

            TextField {
                id: searchField

                Layout.fillWidth: true
                Layout.preferredHeight: 30
                placeholderText: qsTr("Search title, publisher, region, language...")
                text: appController.library.filterText
                selectByMouse: true
                onTextEdited: appController.library.filterText = text
            }

            ToolButton {
                id: clearSearchButton

                text: "×"
                font.pixelSize: 16
                implicitWidth: 30
                implicitHeight: 30
                visible: appController.library.filterText !== ""
                onClicked: appController.library.filterText = ""
                ToolTip.visible: hovered
                ToolTip.text: qsTr("Clear search")
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

        Item {
            Layout.fillWidth: true
            Layout.fillHeight: true

            StackLayout {
                anchors.fill: parent
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
                        visible: !appController.library.loading && gbList.count === 0
                        filtering: appController.library.filterText.trim() !== ""
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
                        visible: !appController.library.loading && gbcList.count === 0
                        filtering: appController.library.filterText.trim() !== ""
                        theme: page.theme
                        platformName: "GBC"
                        onAddRequested: page.addGameRequested()
                        onSettingsRequested: page.settingsRequested("GBC")
                    }
                }
            }

            Rectangle {
                anchors.fill: parent
                visible: appController.library.loading
                color: page.theme.withAlpha(page.theme.window, 0.88)
                z: 10

                ColumnLayout {
                    anchors.centerIn: parent
                    spacing: 8

                    BusyIndicator {
                        Layout.alignment: Qt.AlignHCenter
                        running: appController.library.loading
                    }

                    Label {
                        Layout.alignment: Qt.AlignHCenter
                        text: qsTr("Scanning game library...")
                        color: page.theme.windowText
                        font.pixelSize: 13
                        font.weight: Font.DemiBold
                    }

                    Label {
                        Layout.alignment: Qt.AlignHCenter
                        text: qsTr("Reading cartridge headers and checksums.")
                        color: page.theme.mutedText
                        font.pixelSize: 10
                    }
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

                    RowLayout {
                        Layout.fillWidth: true
                        spacing: 6

                        Label {
                            Layout.fillWidth: true
                            text: model.title
                            color: page.theme.windowText
                            font.pixelSize: 13
                            font.weight: Font.DemiBold
                            elide: Text.ElideRight
                        }

                        Rectangle {
                            id: statusBadge

                            visible: model.status !== "" && model.status !== "unknown"
                                && page.statusLabel(model.status) !== ""
                            Layout.alignment: Qt.AlignVCenter
                            implicitWidth: statusBadgeLabel.implicitWidth + 12
                            implicitHeight: 18
                            radius: 9
                            color: page.statusColor(model.status)

                            Label {
                                id: statusBadgeLabel

                                anchors.centerIn: parent
                                text: page.statusLabel(model.status)
                                color: page.statusTextColor(model.status)
                                font.pixelSize: 9
                                font.weight: Font.DemiBold
                            }
                        }
                    }

                    Label {
                        Layout.fillWidth: true
                        text: model.missing
                            ? qsTr("File missing")
                            : model.publisher + "  ·  " + model.region
                                + (model.languages ? "  ·  " + model.languages : "")
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
                            text: qsTr("Game information")
                            onTriggered: {
                                infoDialog.game = {
                                    title: model.title,
                                    databaseName: model.databaseName,
                                    archiveEntry: model.archiveEntry,
                                    developer: model.developer,
                                    publisher: model.publisher,
                                    region: model.region,
                                    languages: model.languages,
                                    languagesInferred: model.languagesInferred,
                                    year: model.year,
                                    genre: model.genre,
                                    serial: model.serial,
                                    status: model.status,
                                    crc32: model.crc32,
                                    sha1: model.sha1
                                };
                                infoDialog.open();
                            }
                        }

                        MenuSeparator {}

                        MenuItem {
                            text: qsTr("Remove from library")
                            onTriggered: appController.removeGame(model.path)
                        }
                    }
                }
            }
        }
    }

    Dialog {
        id: infoDialog

        property var game: null

        anchors.centerIn: Overlay.overlay
        modal: true
        padding: 16
        width: Math.min(500, page.width - 40)
        title: game ? game.title : qsTr("Game information")
        standardButtons: Dialog.Close

        function rows() {
            if (!game) {
                return [];
            }
            const status = !game.status || game.status === "unknown"
                ? qsTr("Not in database")
                : page.statusLabel(game.status);
            const languages = game.languages
                ? game.languages + (game.languagesInferred ? qsTr(" (inferred)") : "")
                : qsTr("Unknown");
            const rows = [
                {
                    label: qsTr("Database name"),
                    value: game.databaseName || qsTr("Not in database")
                },
                {
                    label: qsTr("Developer"),
                    value: game.developer || qsTr("Unknown")
                },
                {
                    label: qsTr("Publisher"),
                    value: game.publisher || qsTr("Unknown")
                },
                {
                    label: qsTr("Region"),
                    value: game.region || qsTr("Unknown")
                },
                {
                    label: qsTr("Languages"),
                    value: languages
                },
                {
                    label: qsTr("Release year"),
                    value: game.year > 0 ? String(game.year) : qsTr("Unknown")
                },
                {
                    label: qsTr("Genre"),
                    value: game.genre || qsTr("Unknown")
                },
                {
                    label: qsTr("Serial"),
                    value: game.serial || qsTr("Unknown")
                },
                {
                    label: qsTr("Status"),
                    value: status
                },
                {
                    label: qsTr("CRC32"),
                    value: game.crc32 || qsTr("Unknown")
                },
                {
                    label: qsTr("SHA-1"),
                    value: game.sha1 || qsTr("Unknown")
                }
            ];
            if (game.archiveEntry) {
                rows.splice(1, 0, {
                    label: qsTr("Archive entry"),
                    value: game.archiveEntry
                });
            }
            return rows;
        }

        contentItem: ColumnLayout {
            spacing: 6

            Repeater {
                model: infoDialog.rows()

                delegate: RowLayout {
                    required property var modelData

                    Layout.fillWidth: true
                    spacing: 10

                    Label {
                        Layout.preferredWidth: 96
                        text: modelData.label
                        color: page.theme.mutedText
                        font.pixelSize: 11
                    }

                    Label {
                        Layout.fillWidth: true
                        text: modelData.value
                        color: page.theme.windowText
                        font.pixelSize: 11
                        wrapMode: Text.WrapAnywhere
                    }
                }
            }

            Label {
                Layout.fillWidth: true
                Layout.topMargin: 4
                text: qsTr("Metadata: %1 · %2 entries")
                    .arg(appController.library.databaseSource)
                    .arg(appController.library.databaseCount)
                color: page.theme.mutedText
                font.pixelSize: 10
                wrapMode: Text.Wrap
            }

            Label {
                Layout.fillWidth: true
                text: qsTr("Source: No-Intro via libretro-database · CC BY-SA 4.0\n%1")
                    .arg(appController.library.databaseLicenseUrl
                        || "https://creativecommons.org/licenses/by-sa/4.0/")
                color: page.theme.faintText
                font.pixelSize: 10
                wrapMode: Text.Wrap
            }
        }
    }
}
