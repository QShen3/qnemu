import QtQuick

/// Small palette wrapper used by the custom parts of the UI.
///
/// Native controls already follow the platform theme; these derived colors
/// keep cards, separators and subtitles consistent in both light and dark mode.
QtObject {
    id: theme

    property var palette: null

    readonly property color window: palette ? palette.window : "#ffffff"
    readonly property color windowText: palette ? palette.windowText : "#000000"
    readonly property color base: palette ? palette.base : "#ffffff"
    readonly property color text: windowText
    readonly property color mutedText: withAlpha(windowText, 0.62)
    readonly property color faintText: withAlpha(windowText, 0.40)
    readonly property color card: base
    readonly property color cardHover: withAlpha(highlight, 0.16)
    readonly property color border: withAlpha(windowText, 0.10)
    readonly property color separator: withAlpha(windowText, 0.08)
    readonly property color sidebar: withAlpha(windowText, 0.035)
    readonly property color hover: withAlpha(windowText, 0.06)
    readonly property color highlight: palette ? palette.highlight : "#007aff"
    readonly property color accent: highlight
    readonly property color accentText: "#ffffff"
    readonly property color warning: "#e0a23a"

    readonly property color gb: "#8bac0f"
    readonly property color gbText: "#14210a"
    readonly property color gbc: "#7a6ad8"
    readonly property color gbcText: "#ffffff"

    function withAlpha(color, alpha) {
        return Qt.rgba(color.r, color.g, color.b, alpha);
    }
}
