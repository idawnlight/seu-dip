import QtQuick

Item {
    id: root

    anchors.fill: parent

    required property string image

    Image {
        id: image
        anchors.fill: parent
        source: root.image
        fillMode: Image.PreserveAspectFit
    }

    // MouseArea {
    //     anchors.fill: parent
    //
    //     onClicked: {
    //         rect.rotation += 45;
    //     }
    // }

    function reload() {
        image.source = root.image + Math.random();
    }
}
