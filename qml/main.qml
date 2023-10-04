//import related modules
import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import QtQuick.Dialogs
import "components"
import dip

// window containing the application
ApplicationWindow {
    height: 720
    // title of the application
    title: "SEU DIP"
    visible: true
    width: 1280

    // menu containing two menu items
    menuBar: MenuBar {
        Menu {
            property var fileDialog: FileDialog {
                id: fileDialog

                onAccepted: {
                    console.log(selectedFile);
                    AppState.loadImage(selectedFile.toString().replace("file://", ""));

                    originImageViewer.reload();
                    processedImageViewer.reload();
                }
            }

            title: "File"

            MenuItem {
                text: "&Open"

                onTriggered: {
                    // console.log("Open action triggered");
                    // AppState.openImage();
                    fileDialog.open()
                }
            }
            MenuItem {
                text: "Exit"

                onTriggered: Qt.quit()
            }
        }
    }

    // Content Area
    ColumnLayout {
        anchors.fill: parent
        spacing: 0

        Item {
            Layout.fillHeight: true
            Layout.fillWidth: true

            RowLayout {
                anchors.fill: parent
                spacing: 0

                Item {
                    Layout.fillHeight: true
                    Layout.fillWidth: true

                    ImageViewer {
                        id: originImageViewer

                        image: "image://cv/origin"
                    }
                }
                Item {
                    Layout.fillHeight: true
                    Layout.fillWidth: true

                    ImageViewer {
                        id: processedImageViewer

                        image: "image://cv/processed"
                    }
                }
            }
        }

        // buttons
        Row {
            id: buttonRow

            Layout.alignment: Qt.AlignRight
            Layout.margins: 16
            Layout.preferredHeight: 32
            spacing: 2

            Button {
                text: "Fourier transform"

                onClicked: {
                    AppState.fourierTrans();
                    originImageViewer.reload();
                    processedImageViewer.reload();
                }
            }
            Button {
                text: "Reset"

                onClicked: {
                    AppState.resetImage();
                    originImageViewer.reload();
                    processedImageViewer.reload();
                }
            }
            // Button {
            //     text: AppState.imgPath
            //
            //     onClicked: {
            //     }
            // }
        }
    }
}