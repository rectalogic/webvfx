import QtQuick
import QtMultimedia

Rectangle {
    width: webvfx.videoSize.width
    height: webvfx.videoSize.height
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
    Component.onCompleted: {
        webvfx.addVideoSink(sourceVideo.videoSink);
        webvfx.addVideoSink(targetVideo.videoSink);
    }
    Connections {
        target: webvfx
        function onRenderRequested(time) {
            animationController.progress = time;
        }
    }
}
