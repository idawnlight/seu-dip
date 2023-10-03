//import related modules
import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import "components"
import dip

//window containing the application
ApplicationWindow {
    height: 720

    //title of the application
    title: "SEU DIP"
    visible: true
    width: 1280

    //menu containing two menu items
    menuBar: MenuBar {
        Menu {
            title: "File"

            MenuItem {
                text: "&Open"

                onTriggered: {
                    console.log("Open action triggered");
                }
            }
            MenuItem {
                text: "Exit"

                onTriggered: Qt.quit()
            }
        }
    }

    //Content Area

    // Rectangle {
    //     anchors.fill: parent
    //
    //     //a button in the middle of the content area
    //     Button {
    //         anchors.horizontalCenter: parent.horizontalCenter
    //         anchors.verticalCenter: parent.verticalCenter
    //         text: AppState.imgPath
    //     }
    // }

    ColumnLayout {
        anchors.fill: parent
        spacing: 0

        Item {
            Layout.fillHeight: true
            Layout.fillWidth: true

            RowLayout {
                spacing: 0
                anchors.fill: parent

                Item {
                    Layout.fillWidth: true
                    Layout.fillHeight: true

                    ImageViewer {
                        image: "image://cv/origin"
                    }
                }
                Item {
                    Layout.fillWidth: true
                    Layout.fillHeight: true

                    // Layout.minimumWidth: 100
                    // Layout.preferredHeight: 100
                    // Layout.preferredWidth: 200
                    // color: 'plum'
                    //
                    // Text {
                    //     anchors.centerIn: parent
                    //     text: parent.width + 'x' + parent.height
                    // }

                    ImageViewer {
                        image: "image://cv/processed"
                    }
                }
            }
        }

        // buttons
        Row {
            id: buttonRow

            Layout.alignment: Qt.AlignRight
            // Layout.fillHeight: true
            Layout.margins: 16
            Layout.preferredHeight: 32
            spacing: 2

            Button {
                text: parent.width + 'x' + parent.height

                onClicked: {
                }
            }
            Button {
                text: "Save Image"

                onClicked: {
                }
            }
            Button {
                text: AppState.imgPath

                onClicked: {
                }
            }
        }
    }
}