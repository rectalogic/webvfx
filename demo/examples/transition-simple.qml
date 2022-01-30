import QtQuick
import org.webvfx.WebVfx 1.0

Rectangle {
    width: webvfx.videoWidth
    height: webvfx.videoHeight
    color: "lightgray"

    Video {
        id: sourceVideo
        imageName: "SourceImage"
        width: parent.width
        height: parent.height
        clip: true
    }
    Video {
        id: targetVideo
        imageName: "TargetImage"
        width: parent.width
        height: parent.height
        clip: true
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
        target: webvfx
        function onRenderRequested(time) {
            animationController.progress = time;
        }
    }
}
