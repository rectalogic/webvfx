import QtQuick 2.0
import org.webvfx.WebVfx 1.0

Rectangle {
    width: webvfx.videoWidth
    height: webvfx.videoHeight

    Video {
        id: sourceVideo
        imageName: "SourceImage"
        width: parent.width
        height: parent.height
        clip: true
        visible: false
        layer.enabled: true
    }
    Video {
        id: targetVideo
        imageName: "TargetImage"
        width: parent.width
        height: parent.height
        clip: true
        visible: false
        layer.enabled: true
    }

    ShaderEffect {
        id: pageCurl
        width: parent.width
        height: parent.height
        property variant sourceTex: sourceVideo
        property variant targetTex: targetVideo
        property real time
        fragmentShader: "pageCurl.frag.qsb"
    }

    Connections {
        target: webvfx
        function onRenderRequested(time) {
            pageCurl.time = time;
        }
    }
}
