import Qt 4.7
import WebVfx 1.0

Rectangle {
    property alias textureImage: capturedText.pixmap
    color: "#00000000"
    effect: Capture { id: capturedText }
    Text {
        text: "Here is the text"
        color: "red"
        font.pixelSize: parent.height / 3
        smooth: true
        width: parent.width
        height: parent.height
    }
}
