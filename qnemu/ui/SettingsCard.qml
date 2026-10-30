import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

/// Rounded group container used on the settings pages.
Rectangle {
    id: card

    property var theme
    property string title: ""
    default property alias contentData: body.data

    Layout.fillWidth: true
    implicitHeight: layout.implicitHeight + 32
    radius: 10
    color: theme.card
    border.width: 1
    border.color: theme.border

    ColumnLayout {
        id: layout

        anchors.left: parent.left
        anchors.right: parent.right
        anchors.top: parent.top
        anchors.margins: 16
        spacing: 12

        Label {
            visible: card.title !== ""
            text: card.title
            color: card.theme.windowText
            font.pixelSize: 12
            font.weight: Font.DemiBold
            Layout.fillWidth: true
        }

        ColumnLayout {
            id: body
            Layout.fillWidth: true
            spacing: 12
        }
    }
}
