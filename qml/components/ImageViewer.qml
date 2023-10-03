import QtQuick

Item {
    anchors.fill: parent

    required property string image

    // Rectangle {
    //     // id: rect
    //     // anchors.centerIn: parent
    //     anchors.fill: parent
    //     color: "blue"
    //     // height: 50
    //     // width: 50
    //
    //     // Text {
    //     //     anchors.centerIn: parent
    //     //     text: parent.width + 'x' + parent.height
    //     // }
    // }

    Image {
        id: rect
        anchors.fill: parent
        source: image
        fillMode: Image.PreserveAspectFit
    }

    MouseArea {
        anchors.fill: parent

        onClicked: {
            rect.rotation += 45;
        }
    }
}
