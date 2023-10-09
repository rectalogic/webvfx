import QtQuick
import QtQuick.Layouts
import QtMultimedia

GridLayout {
    id: grid
    columns: 2

    VideoOutput {
        id: tlVideo
        Layout.column: 0
        Layout.row: 0
        Layout.fillWidth: true
        Layout.fillHeight: true
    }
    VideoOutput {
        id: trVideo
        Layout.column: 1
        Layout.row: 0
        Layout.fillWidth: true
        Layout.fillHeight: true
    }
    VideoOutput {
        id: blVideo
        Layout.column: 0
        Layout.row: 1
        Layout.fillWidth: true
        Layout.fillHeight: true
    }
    VideoOutput {
        id: brVideo
        Layout.column: 1
        Layout.row: 1
        Layout.fillWidth: true
        Layout.fillHeight: true
    }
    Component.onCompleted: {
        var videoSource = webvfx.addVideoSource();
        webvfx.appendVideoSink(videoSource, tlVideo.videoSink);
        webvfx.appendVideoSink(videoSource, trVideo.videoSink);
        webvfx.appendVideoSink(videoSource, blVideo.videoSink);
        webvfx.appendVideoSink(videoSource, brVideo.videoSink);
    }
}