//import related modules
import QtQuick
import QtQuick.Controls

//window containing the application
ApplicationWindow {
    height: 480

    //title of the application
    title: "Hello World"
    visible: true
    width: 640

    //menu containing two menu items
    menuBar: MenuBar {
        Menu {
            title: "File"

            MenuItem {
                text: "&Open"

                onTriggered: console.log("Open action triggered")
            }
            MenuItem {
                text: "Exit"

                onTriggered: Qt.quit()
            }
        }
    }

    //Content Area

    //a button in the middle of the content area
    Button {
        anchors.horizontalCenter: parent.horizontalCenter
        anchors.verticalCenter: parent.verticalCenter
        text: "Hello World"
    }
}