import QtQuick

Rectangle {
    id: root

    required property string image

    function reload() {
        image.source = root.image + Math.random();
    }

    anchors.fill: parent
    clip: true
    color: "grey"

    Image {
        id: image

        // anchors.centerIn: parent
        // anchors.fill: parent
        fillMode: Image.PreserveAspectFit
        source: root.image
        height: parent.height
        width: parent.width
        x: parent.width / 2 - width / 2
        y: parent.height / 2 - height / 2
    }
    MouseArea {
        anchors.fill: parent
        drag.target: image

        onWheel: {
            if (wheel.angleDelta.y > 0) {
                image.scale = image.scale * 1.1;
            } else {
                image.scale = image.scale / 1.1;
            }
        }
    }
}
