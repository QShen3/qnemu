import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

Dialog {
    id: dialog

    property string firstPath: ""
    property string secondPath: ""
    property string firstTitle: ""
    property string secondTitle: ""
    property var firstOptions: []
    property var secondOptions: []
    property var theme: null
    property var controller: null

    readonly property color textColor: theme ? theme.windowText : "#000000"
    readonly property color mutedColor: theme ? theme.mutedText : "#666666"
    readonly property color faintColor: theme ? theme.faintText : "#888888"
    readonly property color separatorColor: theme ? theme.separator : "#22000000"

    anchors.centerIn: Overlay.overlay
    modal: true
    padding: 18
    width: Math.min(560, (Overlay.overlay ? Overlay.overlay.width : 720) - 64)
    title: dialog.firstTitle !== "" && dialog.secondTitle !== ""
        ? qsTr("Link Play: %1 + %2")
            .arg(dialog.firstTitle)
            .arg(dialog.secondTitle)
        : qsTr("Link Play")

    function openForGames(firstPath, secondPath, firstTitle, secondTitle) {
        dialog.firstPath = firstPath;
        dialog.secondPath = secondPath;
        dialog.firstTitle = firstTitle;
        dialog.secondTitle = secondTitle;
        dialog.firstOptions = controller.saveSlots(firstPath);
        dialog.secondOptions = controller.saveSlots(secondPath);

        firstSaveCombo.currentIndex = 0;
        secondSaveCombo.currentIndex =
            dialog.savePathsConflict(0, 0) ? 2 : 0;
        open();
    }

    function savePathsConflict(firstIndex, secondIndex) {
        if (!firstOptions || !secondOptions
            || firstIndex < 0 || secondIndex < 0
            || firstIndex >= firstOptions.length
            || secondIndex >= secondOptions.length) {
            return false;
        }
        return firstOptions[firstIndex].path === secondOptions[secondIndex].path;
    }

    function currentConflict() {
        return dialog.savePathsConflict(firstSaveCombo.currentIndex,
            secondSaveCombo.currentIndex);
    }

    function optionDescription(options, index) {
        if (!options || index < 0 || index >= options.length) {
            return "";
        }
        const option = options[index];
        const status = option.exists
            ? qsTr("existing save, %1").arg(option.modified)
            : qsTr("new save");
        return option.path + "\n" + status;
    }

    function startGame() {
        if (dialog.currentConflict()) {
            return;
        }
        controller.startLinkGame(dialog.firstPath, dialog.secondPath,
            firstSaveCombo.currentIndex, secondSaveCombo.currentIndex);
        dialog.accept();
    }

    contentItem: ColumnLayout {
        spacing: 10

        Label {
            Layout.fillWidth: true
            text: qsTr("Choose the save each player uses. “Game save” is the same file used in single-player, so progress carries over. Use a “Link save” for a second player when both players share the same ROM.")
            color: dialog.textColor
            font.pixelSize: 11
            wrapMode: Text.Wrap
        }

        Label {
            Layout.fillWidth: true
            Layout.topMargin: 2
            text: qsTr("Player 1 · %1").arg(dialog.firstTitle)
            color: dialog.textColor
            font.pixelSize: 12
            font.weight: Font.DemiBold
            elide: Text.ElideRight
        }

        ComboBox {
            id: firstSaveCombo

            Layout.fillWidth: true
            model: dialog.firstOptions.map(function (option) {
                return option.label;
            })
        }

        Label {
            Layout.fillWidth: true
            text: dialog.optionDescription(dialog.firstOptions,
                firstSaveCombo.currentIndex)
            color: dialog.mutedColor
            font.pixelSize: 10
            wrapMode: Text.WrapAnywhere
        }

        Rectangle {
            Layout.fillWidth: true
            Layout.topMargin: 2
            Layout.bottomMargin: 2
            Layout.preferredHeight: 1
            implicitHeight: 1
            color: dialog.separatorColor
        }

        Label {
            Layout.fillWidth: true
            text: qsTr("Player 2 · %1").arg(dialog.secondTitle)
            color: dialog.textColor
            font.pixelSize: 12
            font.weight: Font.DemiBold
            elide: Text.ElideRight
        }

        ComboBox {
            id: secondSaveCombo

            Layout.fillWidth: true
            model: dialog.secondOptions.map(function (option) {
                return option.label;
            })
        }

        Label {
            Layout.fillWidth: true
            text: dialog.optionDescription(dialog.secondOptions,
                secondSaveCombo.currentIndex)
            color: dialog.mutedColor
            font.pixelSize: 10
            wrapMode: Text.WrapAnywhere
        }

        Label {
            Layout.fillWidth: true
            Layout.topMargin: 4
            visible: dialog.currentConflict()
            text: qsTr("Both players would use the same save file. Choose a different save for one of them.")
            color: "#e5534b"
            font.pixelSize: 11
            wrapMode: Text.Wrap
        }

        Label {
            Layout.fillWidth: true
            text: qsTr("Two games can share the same ROM. In that case give Player 2 a link save, or use a different game for Player 2.")
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
            enabled: !dialog.currentConflict()
                && dialog.firstOptions.length > 0
                && dialog.secondOptions.length > 0
            onClicked: dialog.startGame()
        }
    }
}
