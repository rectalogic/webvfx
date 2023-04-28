import QtQuick
import QtMultimedia

Rectangle {
    width: webvfx.videoSize.width
    height: webvfx.videoSize.height
    color: "lightgray"

    VideoOutput {
        id: video
        anchors.fill: parent
    }
    Text {
        id: timeText
        anchors.verticalCenter: parent.verticalCenter
        anchors.horizontalCenter: parent.horizontalCenter
        font.pointSize: 24
        font.bold: true
    }
    Component.onCompleted: {
        webvfx.addVideoSink(video.videoSink);
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
