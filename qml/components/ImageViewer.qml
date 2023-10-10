import QtQuick

Rectangle {
    id: root

    property bool dragable: true
    property bool fill: false
    required property string image

    function reload() {
        image.source = root.image + Math.random();
    }
    function reset() {
        image.scale = 1;
    }

    anchors.fill: parent
    clip: true
    color: "grey"

    Image {
        id: image

        // anchors.centerIn: parent
        anchors.fill: root.fill ? parent : undefined
        fillMode: root.fill ? Image.Stretch : Image.PreserveAspectFit
        height: parent.height
        source: root.image
        width: parent.width
        x: parent.width / 2 - width / 2
        y: parent.height / 2 - height / 2
    }
    MouseArea {
        anchors.fill: parent
        drag.target: root.dragable ? image : undefined

        onWheel: wheel => {
            if (!root.dragable)
                return;
            if (wheel.angleDelta.y > 0) {
                image.scale = image.scale * 1.1;
            } else {
                image.scale = image.scale / 1.1;
            }
        }
    }
}
