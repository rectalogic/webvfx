import QtQuick
import QtMultimedia

Rectangle {
    id: root
    color: "lightgray"

    VideoOutput {
        id: sourceVideo
        width: parent.width
        height: parent.height
    }
    VideoOutput {
        id: targetVideo
        width: parent.width
        height: parent.height
    }
    ParallelAnimation {
        id: animation
        NumberAnimation {
            property: "x"
            target: sourceVideo
            from: 0
            to: -sourceVideo.width
            duration: 1000
        }
        NumberAnimation {
            property: "x"
            target: targetVideo
            from: targetVideo.width
            to: 0
            duration: 1000
        }
    }
    AnimationController {
        id: animationController
        animation: animation
    }
    Connections {
        target: root
        function onWidthChanged(w) {
            animationController.reload();
        }
        function onHeightChanged(h) {
            animationController.reload();
        }
    }
    Component.onCompleted: {
        webvfx.appendVideoSink(webvfx.addVideoSource(), sourceVideo.videoSink);
        webvfx.appendVideoSink(webvfx.addVideoSource(), targetVideo.videoSink);
    }
    Connections {
        target: webvfx
        function onRenderRequested(time) {
            animationController.progress = time;
        }
    }
}
