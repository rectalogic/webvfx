import QtQuick
import org.webvfx.WebVfx 1.0

Rectangle {
    width: 576;
    height: 432
    color: "lightgray"

    Video {
        id: sourceVideo
        imageName: "SourceImage"
        imageType: webvfx.SourceImageType
        width: parent.width
        height: parent.height
        clip: true
    }
    Video {
        id: targetVideo
        imageName: "TargetImage"
        imageType: webvfx.TargetImageType
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
            duration: 100
        }
        NumberAnimation {
            property: "x"
            target: targetVideo
            from: targetVideo.width
            to: 0
            duration: 100
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
