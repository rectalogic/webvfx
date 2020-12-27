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
        NumberAnimation on x {
            id: sourceAnim
            running: false
            from: 0
            to: -sourceVideo.width
            duration: 100
        }
    }
    Video {
        id: targetVideo
        imageName: "TargetImage"
        imageType: webvfx.TargetImageType
        width: parent.width
        height: parent.height
        clip: true
        NumberAnimation on x {
            id: targetAnim
            running: false
            from: targetVideo.width
            to: 0
            duration: 100
        }
    }
    AnimationController {
        id: sourceController
        animation: sourceAnim
    }
    AnimationController {
        id: targetController
        animation: targetAnim
    }
    Connections {
        target: webvfx
        function onRenderRequested(time) {
            sourceController.progress = time;
            targetController.progress = time;
        }
    }
}
