import QtQuick
import QtMultimedia
import org.webvfx.WebVfx 1.0

Rectangle {
    width: webvfx.videoSize.width
    height: webvfx.videoSize.height

    VideoOutput {
        id: video
        anchors.fill: parent

        Rectangle {
            id: banner
            anchors { horizontalCenter: parent.horizontalCenter; bottom: parent.bottom }
            color: Qt.rgba(0.4, 0.4, 0.4, 0.8)
            width: parent.width
            height: text.height
            Text {
                id: text
                padding: 20
                text: webvfx.getStringParameter("Title") || "Demo Title"
                font.pointSize: 32
                color: "white"
            }
        }
    }

    AnimationController {
        id: animationController
        animation: SequentialAnimation {
            NumberAnimation {
                target: banner
                property: "opacity"
                from: 0
                to: 1
                duration: webvfx.getNumberParameter("FadeIn")
            }
            PauseAnimation {
                duration: 100 - (webvfx.getNumberParameter("FadeIn") + webvfx.getNumberParameter("FadeOut"))
            }
            NumberAnimation {
                target: banner
                property: "opacity"
                from: 1
                to: 0
                duration: webvfx.getNumberParameter("FadeOut")
            }
        }
    }
    Component.onCompleted: {
        webvfx.addVideoSink(video.videoSink);
    }
    Connections {
        target: webvfx
        function onRenderRequested(time) {
            animationController.progress = time;
        }
    }
}
