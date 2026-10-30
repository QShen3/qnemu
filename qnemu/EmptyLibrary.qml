import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

/// Centered empty-library state with a small cartridge illustration.
ColumnLayout {
    id: empty

    property var theme
    property string platformName: "GB"

    signal addRequested
    signal settingsRequested

    spacing: 8

    Item {
        Layout.alignment: Qt.AlignHCenter
        Layout.preferredWidth: 52
        Layout.preferredHeight: 62

        Rectangle {
            anchors.fill: parent
            radius: 6
            color: empty.theme.withAlpha(empty.theme.windowText, 0.06)
            border.width: 1
            border.color: empty.theme.border

            Rectangle {
                anchors.top: parent.top
                anchors.topMargin: 8
                anchors.horizontalCenter: parent.horizontalCenter
                width: parent.width - 18
                height: 20
                radius: 3
                color: empty.theme.withAlpha(empty.theme.windowText, 0.10)

                Label {
                    anchors.centerIn: parent
                    text: empty.platformName
                    color: empty.theme.mutedText
                    font.pixelSize: 8
                    font.bold: true
                }
            }

            Rectangle {
                anchors.bottom: parent.bottom
                anchors.horizontalCenter: parent.horizontalCenter
                width: 20
                height: 5
                radius: 2
                color: empty.theme.window
            }
        }
    }

    Label {
        Layout.alignment: Qt.AlignHCenter
        text: qsTr("No %1 games yet").arg(empty.platformName)
        color: empty.theme.windowText
        font.pixelSize: 15
        font.weight: Font.DemiBold
    }

    Label {
        Layout.alignment: Qt.AlignHCenter
        Layout.maximumWidth: 260
        Layout.fillWidth: true
        text: qsTr("Add a ROM file, or choose a folder in Settings.")
        color: empty.theme.mutedText
        font.pixelSize: 11
        wrapMode: Text.WordWrap
        horizontalAlignment: Text.AlignHCenter
    }

    RowLayout {
        Layout.alignment: Qt.AlignHCenter
        Layout.topMargin: 2
        spacing: 8

        Button {
            id: addButton

            text: qsTr("Add Game...")
            leftPadding: 14
            rightPadding: 14
            topPadding: 7
            bottomPadding: 7
            background: Rectangle {
                radius: 7
                color: addButton.pressed ? Qt.darker(empty.theme.accent, 1.2) : addButton.hovered ? Qt.lighter(empty.theme.accent, 1.15) : empty.theme.accent
            }
            contentItem: Label {
                text: addButton.text
                color: empty.theme.accentText
                font.pixelSize: 12
                font.weight: Font.DemiBold
                horizontalAlignment: Text.AlignHCenter
                verticalAlignment: Text.AlignVCenter
            }
            onClicked: empty.addRequested()
        }

        Button {
            id: settingsButton

            text: qsTr("Settings")
            leftPadding: 14
            rightPadding: 14
            topPadding: 7
            bottomPadding: 7
            background: Rectangle {
                radius: 7
                color: settingsButton.pressed ? empty.theme.hover : settingsButton.hovered ? empty.theme.withAlpha(empty.theme.windowText, 0.09) : "transparent"
                border.width: 1
                border.color: empty.theme.border
            }
            contentItem: Label {
                text: settingsButton.text
                color: empty.theme.windowText
                font.pixelSize: 12
                horizontalAlignment: Text.AlignHCenter
                verticalAlignment: Text.AlignVCenter
            }
            onClicked: empty.settingsRequested()
        }
    }
}
