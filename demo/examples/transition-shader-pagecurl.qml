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
        fragmentShader: "../../build/pageCurl.qsb"
    }

    Component.onCompleted: {
        webvfx.appendVideoSink(webvfx.addVideoSource(), sourceVideo.videoSink);
        webvfx.appendVideoSink(webvfx.addVideoSource(), targetVideo.videoSink);
    }

    Connections {
        target: webvfx
        function onRenderRequested(time) {
            pageCurl.time = time;
        }
    }
}
