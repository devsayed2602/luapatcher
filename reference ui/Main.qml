import QtQuick 2.15
import QtQuick.Layouts 1.15
import QtQuick.Controls 2.15
import QtGraphicalEffects 1.15

ApplicationWindow {
    id: window
    visible: true
    width: 1280
    height: 800
    title: qsTr("Ignite Gaming Dashboard")
    color: "#000000"

    // Custom Palette
    readonly property color colorAccent: "#EFECE3"
    readonly property color colorMuted: "#8FABD4"
    readonly property color colorCard: "#4A70A9"
    readonly property color colorBg: "#000000"

    RowLayout {
        anchors.fill: parent
        spacing: 0

        // SIDEBAR
        Rectangle {
            Layout.fillHeight: true
            width: 260
            color: colorBg
            border.color: Qt.rgba(1, 1, 1, 0.05)
            border.width: 1

            ColumnLayout {
                anchors.fill: parent
                anchors.margins: 30
                spacing: 20

                RowLayout {
                    spacing: 12
                    Rectangle {
                        width: 40; height: 40; radius: 10; color: colorAccent
                        Text { text: "🔥"; anchors.centerIn: parent; font.pixelSize: 20 }
                    }
                    Text {
                        text: "IGNITE"
                        color: "white"
                        font.pixelSize: 22
                        font.bold: true
                        letterSpacing: -1
                    }
                }

                Item { Layout.preferredHeight: 40 } // Spacer

                SidebarItem { label: "Home"; active: true; Layout.fillWidth: true }
                SidebarItem { label: "Games"; Layout.fillWidth: true }
                SidebarItem { label: "Stats"; Layout.fillWidth: true }
                SidebarItem { label: "Store"; Layout.fillWidth: true }

                Item { Layout.fillHeight: true }
            }
        }

        // MAIN
        StackLayout {
            Layout.fillWidth: true
            Layout.fillHeight: true
            currentIndex: 0

            ScrollView {
                clip: true
                ColumnLayout {
                    width: parent.width - 60
                    anchors.horizontalCenter: parent.horizontalCenter
                    anchors.topMargin: 40
                    spacing: 40

                    // Trending Section
                    Rectangle {
                        Layout.fillWidth: true
                        height: 400
                        radius: 25
                        clip: true
                        color: "#222"

                        Image {
                            source: "https://picsum.photos/seed/qt-gaming/1200/600"
                            anchors.fill: parent
                            fillMode: Image.PreserveAspectCrop
                        }

                        Rectangle {
                            anchors.fill: parent
                            gradient: Gradient {
                                GradientStop { position: 0.3; color: "transparent" }
                                GradientStop { position: 1.0; color: colorBg }
                            }
                        }

                        Column {
                            anchors.bottom: parent.bottom
                            anchors.left: parent.left
                            anchors.margins: 40
                            spacing: 10

                            Text { text: "SEASON 4 OUT NOW"; color: colorAccent; font.bold: true; font.pixelSize: 14 }
                            Text { text: "AETHER RECKONING"; color: "white"; font.bold: true; font.pixelSize: 48 }
                            
                            Button {
                                text: "PLAY NOW"
                                background: Rectangle {
                                    radius: 12
                                    color: colorAccent
                                }
                                contentItem: Text {
                                    text: parent.text
                                    color: colorBg
                                    font.bold: true
                                    horizontalAlignment: Text.AlignHCenter
                                }
                            }
                        }
                    }

                    // Store Section
                    Text { text: "GAME STORE"; color: "white"; font.pixelSize: 24; font.bold: true }

                    GridLayout {
                        columns: 3
                        Layout.fillWidth: true
                        columnSpacing: 25
                        rowSpacing: 25

                        GameCard { title: "Cyber Drift"; price: "$49.99" }
                        GameCard { title: "Void Weaver"; price: "FREE" }
                        GameCard { title: "Steel Vanguard"; price: "$34.99" }
                    }
                }
            }
        }

        // RIGHT PANEL
        Rectangle {
            Layout.fillHeight: true
            width: 320
            color: Qt.rgba(1, 1, 1, 0.02)
            border.color: Qt.rgba(1, 1, 1, 0.05)

            ColumnLayout {
                anchors.fill: parent
                anchors.margins: 25
                spacing: 30

                // Profile
                Rectangle {
                    Layout.fillWidth: true
                    height: 120
                    radius: 20
                    color: Qt.rgba(1, 1, 1, 0.05)
                    
                    RowLayout {
                        anchors.fill: parent
                        anchors.margins: 20
                        spacing: 15
                        Rectangle {
                            width: 60; height: 60; radius: 30; color: "gray"
                            clip: true
                            Image { source: "https://picsum.photos/seed/avatar/128/128"; anchors.fill: parent }
                        }
                        Column {
                            Text { text: "Xenon_Hunter"; color: "white"; font.bold: true; font.pixelSize: 18 }
                            Text { text: "Level 42"; color: colorMuted; font.pixelSize: 12 }
                        }
                    }
                }

                Text { text: "LATEST ACTIVITY"; color: "white"; font.pixelSize: 12; font.bold: true; letterSpacing: 2 }

                ListView {
                    Layout.fillWidth: true
                    Layout.fillHeight: true
                    model: 5
                    delegate: RowLayout {
                        width: parent.width; height: 50; spacing: 12
                        Rectangle { width: 32; height: 32; radius: 8; color: "#333" }
                        Column {
                            Text { text: "Friend joined a match"; color: "white"; font.pixelSize: 12 }
                            Text { text: "10m ago"; color: colorMuted; font.pixelSize: 10 }
                        }
                    }
                }
            }
        }
    }
}
