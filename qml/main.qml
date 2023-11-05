//import related modules
import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import QtQuick.Dialogs
import "components"
import dip

// window containing the application
ApplicationWindow {
    id: window
    height: 720
    // title of the application
    title: "SEU DIP"
    visible: true
    width: 1280

    function reload() {
        originImageViewer.reload();
        processedImageViewer.reload();
        originHistogram.reload();
        processedHistogram.reload();
    }

    // menu containing two menu items
    menuBar: MenuBar {
        Menu {
            property var fileDialog: FileDialog {
                id: fileDialog

                property var flag: "grayscale"

                onAccepted: {
                    console.log(selectedFile);
                    AppState.loadImage(selectedFile.toString().replace("file://", ""), flag);
                    window.reload();
                }
            }

            title: "File"

            MenuItem {
                text: "&Open (Color)"

                onTriggered: {
                    fileDialog.flag = "color";
                    fileDialog.open();
                }
            }
            MenuItem {
                text: "&Open (Grayscale)"

                onTriggered: {
                    fileDialog.flag = "grayscale";
                    fileDialog.open();
                }
            }
            MenuItem {
                text: "Black Image"

                onTriggered: {
                    AppState.blackImage();
                    window.reload();
                }
            }
            MenuItem {
                text: "Exit"

                onTriggered: Qt.quit()
            }
        }
        Menu {
            title: "Transform"

            MenuItem {
                text: "Fourier transform"

                onTriggered: {
                    AppState.fourierTrans();
                    window.reload();
                }
            }
        }
        Menu {
            title: "Histogram"

            MenuItem {
                text: "HE (Gray)"

                onTriggered: {
                    AppState.histogramEqualization("Gray");
                    window.reload();
                }
            }
            MenuItem {
                text: "HE (YUV)"

                onTriggered: {
                    AppState.histogramEqualization("YUV");
                    window.reload();
                }
            }
            MenuItem {
                text: "HE (Custom, YUV)"

                onTriggered: {
                    AppState.histogramEqualization("Custom");
                    window.reload();
                }
            }
            MenuItem {
                text: "CLAHE (Gray)"

                onTriggered: {
                    AppState.applyCLAHE("Gray");
                    window.reload();
                }
            }
            MenuItem {
                text: "CLAHE (YUV)"

                onTriggered: {
                    AppState.applyCLAHE("YUV");
                    window.reload();
                }
            }
            MenuItem {
                text: "CLAHE (Custom, YUV)"

                onTriggered: {
                    AppState.applyCLAHE("Custom");
                    window.reload();
                }
            }
        }
        Menu {
            title: "Noise"

            MenuItem {
                text: "Gaussian Noise"

                onTriggered: {
                    AppState.gaussianNoise();
                    window.reload();
                }
            }
            MenuItem {
                text: "Salt and Pepper Noise"

                onTriggered: {
                    AppState.saltAndPepperNoise();
                    window.reload();
                }
            }
        }
        Menu {
            title: "Filter"

            MenuItem {
                text: "Median Filter"

                onTriggered: {
                    AppState.medianFilter();
                    window.reload();
                }
            }
            MenuItem {
                text: "Mean Filter"

                onTriggered: {
                    AppState.meanFilter();
                    window.reload();
                }
            }
            MenuItem {
                text: "Adaptive Median Filter"

                onTriggered: {
                    AppState.adaptiveMedianFilter();
                    window.reload();
                }
            }
            MenuItem {
                text: "Non-local Means Filter"

                onTriggered: {
                    AppState.nonLocalMeanFilter();
                    window.reload();
                }
            }
        }
    }

    // Content Area
    ColumnLayout {
        anchors.fill: parent
        spacing: 0

        Rectangle {
            Layout.fillHeight: true
            Layout.fillWidth: true
            color: "black"

            RowLayout {
                anchors.fill: parent
                spacing: 1

                Item {
                    Layout.fillHeight: true
                    Layout.fillWidth: true

                    Item {
                        height: parent.height - 100
                        width: parent.width

                        ImageViewer {
                            id: originImageViewer

                            image: "image://cv/origin"
                        }
                    }
                    Item {
                        anchors.bottom: parent.bottom
                        height: 100
                        width: parent.width

                        ImageViewer {
                            id: originHistogram

                            dragable: false
                            fill: true
                            image: "image://cv/histogram/origin"
                        }
                    }
                }
                Item {
                    Layout.fillHeight: true
                    Layout.fillWidth: true

                    Item {
                        height: parent.height - 100
                        width: parent.width

                        ImageViewer {
                            id: processedImageViewer

                            image: "image://cv/processed"
                        }
                    }
                    Item {
                        anchors.bottom: parent.bottom
                        height: 100
                        width: parent.width

                        ImageViewer {
                            id: processedHistogram

                            dragable: false
                            fill: true
                            image: "image://cv/histogram/processed"
                        }
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

            Button {
                text: "Swap"

                onClicked: {
                    AppState.swapImage();
                    window.reload();
                }
            }

            Button {
                text: "Reset"

                onClicked: {
                    AppState.resetImage();
                    originImageViewer.reset();
                    processedImageViewer.reset();
                    window.reload();
                }
            }
        }
    }
}