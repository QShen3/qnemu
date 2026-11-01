import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

Dialog {
    id: dialog

    property var playerPaths: []
    property var playerTitles: []
    property int playerCount: 0
    property var playerOptions: []
    property var selectedSlots: []
    property bool infrared: false
    property var theme: null
    property var controller: null

    readonly property color textColor: theme ? theme.windowText : "#000000"
    readonly property color mutedColor: theme ? theme.mutedText : "#666666"
    readonly property color faintColor: theme ? theme.faintText : "#888888"
    readonly property color separatorColor: theme ? theme.separator : "#22000000"

    anchors.centerIn: Overlay.overlay
    modal: true
    padding: 18
    width: Math.min(600, (Overlay.overlay ? Overlay.overlay.width : 800) - 64)
    title: dialog.infrared
        ? qsTr("Infrared Link (%1 players)").arg(dialog.playerCount)
        : (dialog.playerCount > 0
            ? qsTr("Link Play (%1 players)").arg(dialog.playerCount)
            : qsTr("Link Play"))

    function openForGames(paths, titles, infrared) {
        // Tear down the old delegates before changing the option arrays so a
        // shrink from 4 to 2 players never leaves an index pointing past the
        // new arrays.
        dialog.playerCount = 0;
        dialog.infrared = infrared === true;
        const activePaths = dialog.infrared ? paths.slice(0, 2) : paths;
        const activeTitles = dialog.infrared ? titles.slice(0, 2) : titles;
        const options = activePaths.map(function (path) {
            return controller.saveSlots(path);
        });
        dialog.playerPaths = activePaths;
        dialog.playerTitles = activeTitles;
        dialog.selectedSlots = dialog.defaultSlots(options, activePaths.length);
        dialog.playerOptions = options;
        // Set the count last so delegates are created with options and slots
        // already populated.
        dialog.playerCount = activePaths.length;
        open();
    }

    function defaultSlots(options, count) {
        const chosen = [];
        const usedPaths = [];
        const candidates = count === 2 ? [0, 2, 1, 3] : [0, 1, 2, 3];

        for (let player = 0; player < count; ++player) {
            let selected = -1;
            for (let index = 0; index < candidates.length; ++index) {
                const slot = candidates[index];
                const playerOptions = options[player];
                if (!playerOptions || slot >= playerOptions.length) {
                    continue;
                }
                if (usedPaths.indexOf(playerOptions[slot].path) < 0) {
                    selected = slot;
                    break;
                }
            }
            if (selected < 0) {
                selected = 0;
            }
            chosen.push(selected);
            const playerOptions = options[player];
            if (playerOptions && playerOptions[selected]) {
                usedPaths.push(playerOptions[selected].path);
            }
        }
        return chosen;
    }

    function setSlot(player, slot) {
        if (player < 0 || player >= dialog.selectedSlots.length) {
            return;
        }
        const updated = dialog.selectedSlots.slice();
        updated[player] = slot;
        dialog.selectedSlots = updated;
    }

    function optionPath(player, slot) {
        const options = dialog.playerOptions[player];
        if (!options || slot < 0 || slot >= options.length) {
            return "";
        }
        return options[slot].path;
    }

    function optionStatus(player, slot) {
        const options = dialog.playerOptions[player];
        if (!options || slot < 0 || slot >= options.length) {
            return "";
        }
        const option = options[slot];
        return option.exists
            ? qsTr("Existing save, %1").arg(option.modified)
            : qsTr("New save");
    }

    function playerConflict(player) {
        const options = dialog.playerOptions[player];
        const slot = dialog.selectedSlots[player];
        if (!options || slot < 0 || slot >= options.length) {
            return false;
        }
        const selectedPath = options[slot].path;
        for (let other = 0; other < dialog.playerCount; ++other) {
            if (other === player) {
                continue;
            }
            const otherOptions = dialog.playerOptions[other];
            const otherSlot = dialog.selectedSlots[other];
            if (otherOptions && otherSlot >= 0 && otherSlot < otherOptions.length
                && otherOptions[otherSlot].path === selectedPath) {
                return true;
            }
        }
        return false;
    }

    function hasConflict() {
        for (let player = 0; player < dialog.playerCount; ++player) {
            if (dialog.playerConflict(player)) {
                return true;
            }
        }
        return false;
    }

    function startGame() {
        if (dialog.hasConflict()) {
            return;
        }
        if (dialog.infrared) {
            controller.startInfraredGame(dialog.playerPaths,
                dialog.selectedSlots);
        } else {
            controller.startLinkGame(dialog.playerPaths, dialog.selectedSlots);
        }
        dialog.accept();
    }

    contentItem: ColumnLayout {
        spacing: 8

        Label {
            Layout.fillWidth: true
            text: dialog.infrared
                ? qsTr("Choose the save each player uses. Infrared link needs two Game Boy Color games; keep both screens close.")
                : qsTr("Choose the save each player uses. “Game save” is the file used in single-player, so progress carries over. Use a “Link save” when several players share the same ROM.")
            color: dialog.textColor
            font.pixelSize: 11
            wrapMode: Text.Wrap
        }

        Repeater {
            model: dialog.playerCount

            delegate: ColumnLayout {
                required property int index

                Layout.fillWidth: true
                spacing: 2

                Label {
                    Layout.fillWidth: true
                    text: qsTr("Player %1 · %2")
                        .arg(index + 1)
                        .arg(dialog.playerTitles[index])
                    color: dialog.textColor
                    font.pixelSize: 12
                    font.weight: Font.DemiBold
                    elide: Text.ElideRight
                }

                ComboBox {
                    Layout.fillWidth: true
                    model: dialog.playerOptions[index]
                        ? dialog.playerOptions[index].map(function (option) {
                            return option.label;
                        })
                        : []
                    currentIndex: dialog.selectedSlots[index] !== undefined
                        ? dialog.selectedSlots[index]
                        : -1
                    onActivated: dialog.setSlot(index, currentIndex)
                }

                Label {
                    Layout.fillWidth: true
                    text: dialog.optionPath(index, dialog.selectedSlots[index])
                    color: dialog.mutedColor
                    font.pixelSize: 10
                    elide: Text.ElideMiddle
                }

                Label {
                    Layout.fillWidth: true
                    text: dialog.optionStatus(index, dialog.selectedSlots[index])
                    color: dialog.faintColor
                    font.pixelSize: 10
                    elide: Text.ElideRight
                }

                Label {
                    Layout.fillWidth: true
                    visible: dialog.playerConflict(index)
                    text: qsTr("This save is also used by another player.")
                    color: "#e5534b"
                    font.pixelSize: 10
                    wrapMode: Text.Wrap
                }
            }
        }

        Label {
            Layout.fillWidth: true
            Layout.topMargin: 2
            text: dialog.infrared
                ? qsTr("Infrared uses the Game Boy Color IR port; both games must be CGB-capable.")
                : qsTr("Four-player games use the DMG-07 adapter protocol; two-player games use a direct link cable.")
            color: dialog.faintColor
            font.pixelSize: 10
            wrapMode: Text.Wrap
        }
    }

    footer: RowLayout {
        width: dialog.width - dialog.padding * 2
        spacing: 8

        Item {
            Layout.fillWidth: true
        }

        Button {
            text: qsTr("Cancel")
            onClicked: dialog.reject()
        }

        Button {
            text: qsTr("Start Link Play")
            enabled: !dialog.hasConflict() && dialog.playerCount >= 2
            onClicked: dialog.startGame()
        }
    }
}
