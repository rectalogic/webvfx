import QtQuick
import org.webvfx.WebVfx 1.0

Rectangle {
    width: webvfx.videoWidth
    height: webvfx.videoHeight
    color: "lightgray"

    Video {
        id: video
        imageName: "SourceImage"
        width: parent.width
        height: parent.height
        clip: true
    }
    Text {
        id: timeText
        anchors.verticalCenter: parent.verticalCenter
        anchors.horizontalCenter: parent.horizontalCenter
        font.pointSize: 24
        font.bold: true
    }
    Connections {
        target: webvfx
        function onRenderRequested(time) {
            video.rotation = time * 360;
	        timeText.text = "rotating " + Math.round(video.rotation);
            timeText.rotation = -time * 360;
	    }
    }
}
