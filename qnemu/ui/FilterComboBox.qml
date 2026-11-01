import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

/// A ComboBox bound to one of GameLibrary's filter values.
///
/// `filterOptions` is an array of { label, value } objects; the first entry is
/// always the "all" value (empty string).
ComboBox {
    id: control

    property string filterValue: ""
    property var filterOptions: []
    property string allLabel: qsTr("All")

    signal selected(string value)

    Layout.fillWidth: true
    Layout.preferredHeight: 30

    model: [{ label: allLabel, value: "" }].concat(filterOptions)
    textRole: "label"
    valueRole: "value"

    onFilterValueChanged: sync()
    onModelChanged: sync()
    Component.onCompleted: sync()
    onActivated: control.selected(currentValue)

    function sync() {
        const index = indexOfValue(control.filterValue);
        control.currentIndex = index >= 0 ? index : 0;
    }
}
