import QtQuick
import QtQuick.Controls

/// Transient bottom-center status message shared by the library and game
/// windows.  It sits on a dark scrim so it stays readable over the game.
Rectangle {
    id: toast

    property string currentText: ""

    function show(message) {
        currentText = message;
        hideTimer.restart();
    }

    anchors.horizontalCenter: parent.horizontalCenter
    anchors.bottom: parent.bottom
    anchors.bottomMargin: 12
    width: Math.min(parent.width - 24, label.implicitWidth + 24)
    height: label.implicitHeight + 12
    radius: 8
    color: "#cc000000"
    visible: currentText.length > 0
    z: 50

    Label {
        id: label

        anchors.centerIn: parent
        width: parent.width - 24
        text: toast.currentText
        color: "white"
        horizontalAlignment: Text.AlignHCenter
        wrapMode: Text.Wrap
    }

    Timer {
        id: hideTimer

        interval: 3500
        onTriggered: toast.currentText = ""
    }
}
