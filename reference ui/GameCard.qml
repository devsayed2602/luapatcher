import QtQuick 2.15
import QtQuick.Layouts 1.15
import QtGraphicalEffects 1.15

Item {
    id: root
    Layout.preferredWidth: 280
    Layout.preferredHeight: 340
    property string title: "Game Title"
    property string price: "$0.00"

    Rectangle {
        id: cardSurface
        anchors.fill: parent
        radius: 20
        color: "#4A70A9"
        // Neumorphic shadow simulation
        border.color: Qt.rgba(1, 1, 1, 0.05)
        
        ColumnLayout {
            anchors.fill: parent
            anchors.margins: 15
            spacing: 12

            Rectangle {
                Layout.fillWidth: true
                Layout.preferredHeight: 180
                radius: 12
                color: "#000"
                clip: true
                Image {
                    source: "https://picsum.photos/seed/" + root.title + "/400/300"
                    anchors.fill: parent
                    fillMode: Image.PreserveAspectCrop
                }
            }

            Column {
                Layout.fillWidth: true
                spacing: 4
                Text { text: root.title; color: "white"; font.pixelSize: 18; font.bold: true }
                Text { text: "Action / Adventure"; color: "#8FABD4"; font.pixelSize: 12 }
            }

            Item { Layout.fillHeight: true } // Spacer

            RowLayout {
                Layout.fillWidth: true
                height: 40
                Text { text: root.price; color: "#EFECE3"; font.bold: true; font.pixelSize: 16; Layout.fillWidth: true }
                Rectangle {
                    width: 32; height: 32; radius: 8; color: Qt.rgba(1, 1, 1, 0.1)
                    Text { text: ">"; color: "white"; anchors.centerIn: parent }
                }
            }
        }
    }

    DropShadow {
        anchors.fill: cardSurface
        horizontalOffset: 5
        verticalOffset: 5
        radius: 15
        samples: 30
        color: "#AA000000"
        source: cardSurface
        z: -1
    }
}
