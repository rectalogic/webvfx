import QtQuick 2.0
import QtMultimedia
import org.webvfx.WebVfx 1.0

Rectangle {
    width: webvfx.videoSize.width
    height: webvfx.videoSize.height

    VideoOutput {
        id: sourceVideo
        anchors.fill: parent
        visible: false
        layer.enabled: true
    }
    VideoOutput {
        id: targetVideo
        anchors.fill: parent
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

    VideoOutput {
        id: extraVideo
        width: parent.width / 4
        height: parent.height / 4
        rotation: 45
        anchors.verticalCenter: parent.verticalCenter
        anchors.horizontalCenter: parent.horizontalCenter
        visible: false
        layer.enabled: true
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
    Component.onCompleted: {
        webvfx.addVideoSink(sourceVideo.videoSink);
        webvfx.addVideoSink(targetVideo.videoSink);
        // webvfx.addVideoSink(extraVideo.videoSink);
    }
    Connections {
        target: webvfx
        function onRenderRequested(time) {
            animationController.progress = time;
        }
    }
}
