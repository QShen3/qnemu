import QtQuick
import QtQuick.Controls
import QtQuick.Dialogs
import QtQuick.Layouts

Dialog {
    id: dialog

    property var theme: null
    property var controller: null
    property string romPath: ""
    property string romTitle: ""
    property var slots: []
    property var backups: []
    property string blockedReason: ""
    property string message: ""
    property int pendingDeleteSlot: -1
    property int pendingCopyFrom: -1
    property int pendingCopyTo: -1
    property int pendingImportSlot: -1
    property url pendingImportUrl: ""
    property int pendingExportSlot: -1
    property int pendingRenameSlot: -1
    property string renameText: ""
    property var pendingRestoreBackup: null
    property var pendingDeleteBackup: null

    readonly property color textColor: theme ? theme.windowText : "#000000"
    readonly property color mutedColor: theme ? theme.mutedText : "#666666"
    readonly property color faintColor: theme ? theme.faintText : "#888888"
    readonly property color separatorColor: theme ? theme.separator : "#22000000"
    readonly property color cardColor: theme ? theme.card : "#ffffff"
    readonly property color warningColor: theme ? theme.warning : "#e0a23a"

    anchors.centerIn: Overlay.overlay
    modal: true
    padding: 18
    width: Math.min(660, (Overlay.overlay ? Overlay.overlay.width : 800) - 64)
    title: dialog.romTitle !== ""
        ? qsTr("Save data: %1").arg(dialog.romTitle)
        : qsTr("Save data")

    function openForGame(path, title) {
        dialog.romPath = path;
        dialog.romTitle = title;
        dialog.message = "";
        dialog.refresh();
        open();
    }

    function refresh() {
        dialog.slots = controller.saveSlots(dialog.romPath);
        dialog.blockedReason = controller.saveDataBlockedReason(dialog.romPath);
        dialog.backups = controller.saveBackups(dialog.romPath);
    }

    function slotAt(index) {
        return index >= 0 && index < slots.length ? slots[index] : null;
    }

    function labelAt(index) {
        const slot = slotAt(index);
        return slot ? slot.label : qsTr("Save %1").arg(index + 1);
    }

    function formatSize(bytes) {
        if (bytes >= 1024 * 1024) {
            return (bytes / (1024 * 1024)).toFixed(1) + " MB";
        }
        if (bytes >= 1024) {
            return (bytes / 1024).toFixed(1) + " KB";
        }
        return bytes + " B";
    }

    function slotStatus(slot) {
        if (!slot) {
            return "";
        }
        if (!slot.exists) {
            return qsTr("Empty");
        }
        return qsTr("Existing save · %1 · %2%3")
            .arg(formatSize(slot.size), slot.modified)
            .arg(slot.rtcExists ? qsTr(" · RTC") : "");
    }

    function playSlot(slot) {
        controller.openGameWithSave(dialog.romPath, slot);
        dialog.accept();
    }

    function requestCopy(from, to) {
        dialog.pendingCopyFrom = from;
        dialog.pendingCopyTo = to;
        const target = slotAt(to);
        if (target && target.exists) {
            copyConfirm.open();
        } else {
            dialog.doCopy();
        }
    }

    function doCopy() {
        controller.copySave(dialog.romPath, dialog.pendingCopyFrom, dialog.pendingCopyTo);
        dialog.message = controller.statusText;
        dialog.refresh();
    }

    function requestDelete(slot) {
        dialog.pendingDeleteSlot = slot;
        deleteConfirm.open();
    }

    function doDelete() {
        controller.deleteSave(dialog.romPath, dialog.pendingDeleteSlot);
        dialog.message = controller.statusText;
        dialog.refresh();
        deleteConfirm.accept();
    }

    function backupSlot(slot) {
        controller.backupSave(dialog.romPath, slot);
        dialog.message = controller.statusText;
        dialog.refresh();
    }

    function requestImport(slot) {
        dialog.pendingImportSlot = slot;
        importDialog.open();
    }

    function doImport() {
        controller.importSave(dialog.romPath, dialog.pendingImportSlot,
            dialog.pendingImportUrl);
        dialog.message = controller.statusText;
        dialog.refresh();
        importConfirm.accept();
    }

    function requestExport(slot) {
        dialog.pendingExportSlot = slot;
        exportDialog.open();
    }

    function closeRunningGame() {
        controller.closeGame();
        dialog.message = controller.statusText;
        dialog.refresh();
    }

    function requestRename(slot) {
        dialog.pendingRenameSlot = slot;
        dialog.renameText = dialog.labelAt(slot);
        renameDialog.open();
    }

    function doRename() {
        controller.renameSaveSlot(dialog.romPath, dialog.pendingRenameSlot,
            dialog.renameText);
        dialog.message = controller.statusText;
        dialog.refresh();
        renameDialog.accept();
    }

    function requestRestore(backup) {
        dialog.pendingRestoreBackup = backup;
        const target = dialog.slotAt(backup.slot);
        if (target && target.exists) {
            restoreConfirm.open();
        } else {
            dialog.doRestore();
        }
    }

    function doRestore() {
        if (!dialog.pendingRestoreBackup) {
            return;
        }
        controller.restoreBackup(dialog.romPath,
            dialog.pendingRestoreBackup.path,
            dialog.pendingRestoreBackup.slot);
        dialog.message = controller.statusText;
        dialog.refresh();
        restoreConfirm.accept();
    }

    function requestDeleteBackup(backup) {
        dialog.pendingDeleteBackup = backup;
        deleteBackupConfirm.open();
    }

    function doDeleteBackup() {
        if (!dialog.pendingDeleteBackup) {
            return;
        }
        controller.deleteBackup(dialog.romPath,
            dialog.pendingDeleteBackup.path);
        dialog.message = controller.statusText;
        dialog.refresh();
        deleteBackupConfirm.accept();
    }

    function openBackups() {
        backupDialog.open();
    }

    contentItem: ColumnLayout {
        spacing: 10

        Label {
            Layout.fillWidth: true
            text: qsTr("“Game save” is used by normal play. “Link save” slots are used when starting link play. You can play any slot, copy between slots, import/export .sav files, or create a backup.")
            color: dialog.textColor
            font.pixelSize: 11
            wrapMode: Text.Wrap
        }

        Rectangle {
            Layout.fillWidth: true
            Layout.preferredHeight: blockedRow.implicitHeight + 16
            visible: dialog.blockedReason !== ""
            radius: 8
            color: dialog.theme
                ? dialog.theme.withAlpha(dialog.warningColor, 0.12)
                : "#22e0a23a"
            border.width: 1
            border.color: dialog.theme
                ? dialog.theme.withAlpha(dialog.warningColor, 0.45)
                : "#66e0a23a"

            RowLayout {
                id: blockedRow

                anchors.fill: parent
                anchors.margins: 8
                spacing: 8

                Label {
                    Layout.fillWidth: true
                    text: dialog.blockedReason === "game"
                        ? qsTr("This game is still running. Close it to flush its save before editing.")
                        : qsTr("Link play is running. Close the Link Play window before editing saves.")
                    color: dialog.textColor
                    font.pixelSize: 11
                    wrapMode: Text.Wrap
                }

                Button {
                    text: qsTr("Close game")
                    visible: dialog.blockedReason === "game"
                    onClicked: dialog.closeRunningGame()
                }

                Button {
                    text: qsTr("Stop link play")
                    visible: dialog.blockedReason === "link"
                    onClicked: {
                        controller.stopLink();
                        dialog.message = qsTr("Link play stopped; saves flushed");
                        dialog.refresh();
                    }
                }
            }
        }

        Repeater {
            model: dialog.slots

            delegate: Rectangle {
                id: card

                required property var modelData

                Layout.fillWidth: true
                implicitHeight: cardLayout.implicitHeight + 24
                radius: 8
                color: dialog.cardColor
                border.width: 1
                border.color: dialog.separatorColor

                ColumnLayout {
                    id: cardLayout

                    anchors.fill: parent
                    anchors.margins: 12
                    spacing: 4

                    RowLayout {
                        Layout.fillWidth: true
                        spacing: 8

                        ColumnLayout {
                            Layout.fillWidth: true
                            spacing: 2

                            Label {
                                Layout.fillWidth: true
                                text: card.modelData.label
                                color: dialog.textColor
                                font.pixelSize: 13
                                font.weight: Font.DemiBold
                            }

                            Label {
                                Layout.fillWidth: true
                                text: card.modelData.path
                                color: dialog.mutedColor
                                font.pixelSize: 10
                                wrapMode: Text.WrapAnywhere
                            }

                            Label {
                                Layout.fillWidth: true
                                text: dialog.slotStatus(card.modelData)
                                color: card.modelData.exists
                                    ? dialog.textColor
                                    : dialog.faintColor
                                font.pixelSize: 10
                            }
                        }

                        Button {
                            text: card.modelData.exists ? qsTr("Play") : qsTr("Start new")
                            enabled: dialog.blockedReason === ""
                            onClicked: dialog.playSlot(card.modelData.slot)
                        }

                        ToolButton {
                            text: "⋯"
                            enabled: dialog.blockedReason === ""
                            onClicked: slotMenu.popup()

                            Menu {
                                id: slotMenu

                                property int sourceSlot: card.modelData.slot
                                property bool sourceExists: card.modelData.exists

                                MenuItem {
                                    text: slotMenu.sourceExists
                                        ? qsTr("Play")
                                        : qsTr("Start new")
                                    onTriggered: dialog.playSlot(slotMenu.sourceSlot)
                                }

                                MenuItem {
                                    text: qsTr("Rename...")
                                    onTriggered: dialog.requestRename(slotMenu.sourceSlot)
                                }

                                MenuSeparator {}

                                MenuItem {
                                    text: qsTr("Copy to %1").arg(dialog.labelAt(0))
                                    enabled: slotMenu.sourceExists
                                        && slotMenu.sourceSlot !== 0
                                    onTriggered: dialog.requestCopy(slotMenu.sourceSlot, 0)
                                }

                                MenuItem {
                                    text: qsTr("Copy to %1").arg(dialog.labelAt(1))
                                    enabled: slotMenu.sourceExists
                                        && slotMenu.sourceSlot !== 1
                                    onTriggered: dialog.requestCopy(slotMenu.sourceSlot, 1)
                                }

                                MenuItem {
                                    text: qsTr("Copy to %1").arg(dialog.labelAt(2))
                                    enabled: slotMenu.sourceExists
                                        && slotMenu.sourceSlot !== 2
                                    onTriggered: dialog.requestCopy(slotMenu.sourceSlot, 2)
                                }

                                MenuSeparator {}

                                MenuItem {
                                    text: qsTr("Back up")
                                    enabled: slotMenu.sourceExists
                                    onTriggered: dialog.backupSlot(slotMenu.sourceSlot)
                                }

                                MenuItem {
                                    text: qsTr("Import...")
                                    onTriggered: dialog.requestImport(slotMenu.sourceSlot)
                                }

                                MenuItem {
                                    text: qsTr("Export...")
                                    enabled: slotMenu.sourceExists
                                    onTriggered: dialog.requestExport(slotMenu.sourceSlot)
                                }

                                MenuSeparator {}

                                MenuItem {
                                    text: qsTr("Delete")
                                    enabled: slotMenu.sourceExists
                                    onTriggered: dialog.requestDelete(slotMenu.sourceSlot)
                                }
                            }
                        }
                    }
                }
            }
        }

        Label {
            Layout.fillWidth: true
            visible: dialog.message !== ""
            text: dialog.message
            color: dialog.mutedColor
            font.pixelSize: 10
            wrapMode: Text.Wrap
        }

        Label {
            Layout.fillWidth: true
            text: qsTr("Backups are written to the “backups” folder inside the save directory.")
            color: dialog.faintColor
            font.pixelSize: 10
            wrapMode: Text.Wrap
        }
    }

    footer: RowLayout {
        width: dialog.width - dialog.padding * 2
        spacing: 8

        Button {
            text: qsTr("Backups (%1)").arg(dialog.backups.length)
            onClicked: dialog.openBackups()
        }

        Item {
            Layout.fillWidth: true
        }

        Button {
            text: qsTr("Close")
            onClicked: dialog.reject()
        }
    }

    Dialog {
        id: deleteConfirm

        anchors.centerIn: Overlay.overlay
        modal: true
        padding: 16
        width: 380
        title: qsTr("Delete save data?")

        contentItem: Label {
            text: qsTr("Delete %1?\nThis cannot be undone.")
                .arg(dialog.labelAt(dialog.pendingDeleteSlot))
            color: dialog.textColor
            wrapMode: Text.Wrap
        }

        footer: RowLayout {
            width: deleteConfirm.width - deleteConfirm.padding * 2
            spacing: 8

            Item {
                Layout.fillWidth: true
            }

            Button {
                text: qsTr("Cancel")
                onClicked: deleteConfirm.reject()
            }

            Button {
                text: qsTr("Delete")
                onClicked: dialog.doDelete()
            }
        }
    }

    Dialog {
        id: copyConfirm

        anchors.centerIn: Overlay.overlay
        modal: true
        padding: 16
        width: 400
        title: qsTr("Overwrite save data?")

        contentItem: Label {
            text: qsTr("Overwrite %1 with %2?\nThe target save will be replaced.")
                .arg(dialog.labelAt(dialog.pendingCopyTo),
                    dialog.labelAt(dialog.pendingCopyFrom))
            color: dialog.textColor
            wrapMode: Text.Wrap
        }

        footer: RowLayout {
            width: copyConfirm.width - copyConfirm.padding * 2
            spacing: 8

            Item {
                Layout.fillWidth: true
            }

            Button {
                text: qsTr("Cancel")
                onClicked: copyConfirm.reject()
            }

            Button {
                text: qsTr("Overwrite")
                onClicked: {
                    dialog.doCopy();
                    copyConfirm.accept();
                }
            }
        }
    }

    Dialog {
        id: importConfirm

        anchors.centerIn: Overlay.overlay
        modal: true
        padding: 16
        width: 400
        title: qsTr("Overwrite save data?")

        contentItem: Label {
            text: qsTr("Overwrite %1 with the selected file?")
                .arg(dialog.labelAt(dialog.pendingImportSlot))
            color: dialog.textColor
            wrapMode: Text.Wrap
        }

        footer: RowLayout {
            width: importConfirm.width - importConfirm.padding * 2
            spacing: 8

            Item {
                Layout.fillWidth: true
            }

            Button {
                text: qsTr("Cancel")
                onClicked: importConfirm.reject()
            }

            Button {
                text: qsTr("Overwrite")
                onClicked: dialog.doImport()
            }
        }
    }

    Dialog {
        id: renameDialog

        anchors.centerIn: Overlay.overlay
        modal: true
        padding: 16
        width: 400
        title: qsTr("Rename save slot")

        contentItem: ColumnLayout {
            spacing: 8

            Label {
                Layout.fillWidth: true
                text: qsTr("Display name for this save slot:")
                color: dialog.textColor
                font.pixelSize: 11
            }

            TextField {
                id: renameField

                Layout.fillWidth: true
                text: dialog.renameText
                maximumLength: 40
                onTextEdited: dialog.renameText = text
                onAccepted: dialog.doRename()
            }

            Label {
                Layout.fillWidth: true
                text: qsTr("Leave it blank to use the default name.")
                color: dialog.faintColor
                font.pixelSize: 10
                wrapMode: Text.Wrap
            }
        }

        onOpened: {
            renameField.forceActiveFocus();
            renameField.selectAll();
        }

        footer: RowLayout {
            width: renameDialog.width - renameDialog.padding * 2
            spacing: 8

            Item {
                Layout.fillWidth: true
            }

            Button {
                text: qsTr("Cancel")
                onClicked: renameDialog.reject()
            }

            Button {
                text: qsTr("Rename")
                onClicked: dialog.doRename()
            }
        }
    }

    Dialog {
        id: restoreConfirm

        anchors.centerIn: Overlay.overlay
        modal: true
        padding: 16
        width: 420
        title: qsTr("Restore backup?")

        contentItem: Label {
            text: dialog.pendingRestoreBackup
                ? qsTr("Restore the %1 backup from %2 into %3?\nThe current save will be replaced.")
                    .arg(dialog.pendingRestoreBackup.slotLabel,
                        dialog.pendingRestoreBackup.timestamp,
                        dialog.labelAt(dialog.pendingRestoreBackup.slot))
                : ""
            color: dialog.textColor
            wrapMode: Text.Wrap
        }

        footer: RowLayout {
            width: restoreConfirm.width - restoreConfirm.padding * 2
            spacing: 8

            Item {
                Layout.fillWidth: true
            }

            Button {
                text: qsTr("Cancel")
                onClicked: restoreConfirm.reject()
            }

            Button {
                text: qsTr("Restore")
                onClicked: dialog.doRestore()
            }
        }
    }

    Dialog {
        id: deleteBackupConfirm

        anchors.centerIn: Overlay.overlay
        modal: true
        padding: 16
        width: 420
        title: qsTr("Delete backup?")

        contentItem: Label {
            text: dialog.pendingDeleteBackup
                ? qsTr("Delete the %1 backup from %2?\nThis cannot be undone.")
                    .arg(dialog.pendingDeleteBackup.slotLabel,
                        dialog.pendingDeleteBackup.timestamp)
                : ""
            color: dialog.textColor
            wrapMode: Text.Wrap
        }

        footer: RowLayout {
            width: deleteBackupConfirm.width - deleteBackupConfirm.padding * 2
            spacing: 8

            Item {
                Layout.fillWidth: true
            }

            Button {
                text: qsTr("Cancel")
                onClicked: deleteBackupConfirm.reject()
            }

            Button {
                text: qsTr("Delete")
                onClicked: dialog.doDeleteBackup()
            }
        }
    }

    Dialog {
        id: backupDialog

        anchors.centerIn: Overlay.overlay
        modal: true
        padding: 16
        width: Math.min(620, (Overlay.overlay ? Overlay.overlay.width : 800) - 80)
        height: Math.min(460, (Overlay.overlay ? Overlay.overlay.height : 700) - 80)
        title: qsTr("Save backups")

        contentItem: ColumnLayout {
            spacing: 8

            Label {
                Layout.fillWidth: true
                text: qsTr("Restoring a backup replaces the save in its original slot.")
                color: dialog.mutedColor
                font.pixelSize: 10
                wrapMode: Text.Wrap
            }

            ListView {
                Layout.fillWidth: true
                Layout.fillHeight: true
                clip: true
                spacing: 6
                model: dialog.backups
                ScrollBar.vertical: ScrollBar {}

                delegate: Rectangle {
                    required property var modelData

                    width: ListView.view.width
                    implicitHeight: backupRow.implicitHeight + 16
                    radius: 6
                    color: dialog.cardColor
                    border.width: 1
                    border.color: dialog.separatorColor

                    RowLayout {
                        id: backupRow

                        anchors.fill: parent
                        anchors.margins: 8
                        spacing: 8

                        ColumnLayout {
                            Layout.fillWidth: true
                            spacing: 1

                            Label {
                                Layout.fillWidth: true
                                text: modelData.slotLabel + " · " + modelData.timestamp
                                color: dialog.textColor
                                font.pixelSize: 12
                                elide: Text.ElideRight
                            }

                            Label {
                                Layout.fillWidth: true
                                text: modelData.fileName + " · "
                                    + dialog.formatSize(modelData.size)
                                color: dialog.mutedColor
                                font.pixelSize: 10
                                elide: Text.ElideMiddle
                            }
                        }

                        Button {
                            text: qsTr("Restore")
                            onClicked: dialog.requestRestore(modelData)
                        }

                        Button {
                            text: qsTr("Delete")
                            onClicked: dialog.requestDeleteBackup(modelData)
                        }
                    }
                }
            }

            Label {
                Layout.fillWidth: true
                visible: dialog.backups.length === 0
                text: qsTr("No backups yet. Use the “Back up” action on a save slot.")
                color: dialog.faintColor
                font.pixelSize: 11
                wrapMode: Text.Wrap
            }
        }

        footer: RowLayout {
            width: backupDialog.width - backupDialog.padding * 2
            spacing: 8

            Item {
                Layout.fillWidth: true
            }

            Button {
                text: qsTr("Close")
                onClicked: backupDialog.reject()
            }
        }
    }

    FileDialog {
        id: importDialog

        title: qsTr("Import .sav file")
        nameFilters: [qsTr("Game Boy save files (*.sav)"), qsTr("All files (*)")]
        fileMode: FileDialog.OpenFile
        onAccepted: {
            dialog.pendingImportUrl = selectedFile;
            const target = dialog.slotAt(dialog.pendingImportSlot);
            if (target && target.exists) {
                importConfirm.open();
            } else {
                dialog.doImport();
            }
        }
    }

    FileDialog {
        id: exportDialog

        title: qsTr("Export .sav file")
        nameFilters: [qsTr("Game Boy save files (*.sav)")]
        fileMode: FileDialog.SaveFile
        defaultSuffix: "sav"
        onAccepted: {
            controller.exportSave(dialog.romPath, dialog.pendingExportSlot,
                selectedFile);
            dialog.message = controller.statusText;
            dialog.refresh();
        }
    }
}
