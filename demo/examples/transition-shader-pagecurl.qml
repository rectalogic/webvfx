import QtQuick 2.0
import QtMultimedia

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
        id: pageCurl
        anchors.fill: parent
        property variant sourceTex: sourceVideo
        property variant targetTex: targetVideo
        property real time
        fragmentShader: "pageCurl.frag.qsb"
    }

    Component.onCompleted: {
        webvfx.addVideoSink(sourceVideo.videoSink);
        webvfx.addVideoSink(targetVideo.videoSink);
    }

    Connections {
        target: webvfx
        function onRenderRequested(time) {
            pageCurl.time = time;
        }
    }
}
