import QtQuick 2.15
import QtQuick.Layouts 1.15
import QtQuick.Controls 2.15

Button {
    id: control
    property string label: "Button"
    property bool active: false

    contentItem: RowLayout {
        spacing: 15
        Rectangle {
            width: 4
            height: 24
            radius: 2
            color: active ? "#EFECE3" : "transparent"
        }
        Text {
            text: control.label
            color: control.active ? "white" : "#8FABD4"
            font.pixelSize: 15
            font.weight: control.active ? Font.Bold : Font.Normal
            Layout.fillWidth: true
        }
    }

    background: Rectangle {
        implicitWidth: 200
        implicitHeight: 45
        color: control.hovered ? Qt.rgba(1, 1, 1, 0.05) : "transparent"
        radius: 10
    }
}
