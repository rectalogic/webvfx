import QtQuick 2.0
import org.webvfx.WebVfx 1.0

Rectangle {
    width: webvfx.videoWidth
    height: webvfx.videoHeight

    Video {
        id: sourceVideo
        imageName: "SourceImage"
        imageType: webvfx.SourceImageType
        width: parent.width
        height: parent.height
        clip: true
        visible: false
        layer.enabled: true
    }
    Video {
        id: targetVideo
        imageName: "TargetImage"
        imageType: webvfx.SourceImageType
        width: parent.width
        height: parent.height
        clip: true
        visible: false
        layer.enabled: true
    }

    ShaderEffect {
        id: crossZoom
        width: parent.width
        height: parent.height
        property variant sourceTex: sourceVideo
        property variant targetTex: targetVideo
        property real dissolve
        property real strength
        property point center: Qt.point(0, 0.5)
        fragmentShader: "crossZoom.frag.qsb"
    }
    ParallelAnimation {
        id: animation
        // Mirrored sinusoidal loop. 0->strength then strength->0
        SequentialAnimation {
            NumberAnimation {
                property: "strength"
                target: crossZoom
                from: 0
                to: webvfx.getNumberParameter("Strength") || 0.3
                duration: 500
                easing.type: Easing.InOutSine
            }
            NumberAnimation {
                property: "strength"
                target: crossZoom
                from: webvfx.getNumberParameter("Strength") || 0.3
                to: 0
                duration: 500
                easing.type: Easing.InOutSine
            }
        }
        NumberAnimation {
            property: "dissolve"
            target: crossZoom
            from: 0
            to: 1
            duration: 1000
            easing.type: Easing.InOutExpo
        }
        // Linear interpolate center across center half of the image
        NumberAnimation {
            property: "center.x"
            target: crossZoom
            from: 0.25
            to: 0.5
            duration: 1000
            easing.type: Easing.Linear
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