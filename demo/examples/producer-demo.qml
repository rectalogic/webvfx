import QtQuick
import QtQuick.Shapes 1.0

Shape {
    id: shape
    width: webvfx.videoWidth
    height: webvfx.videoHeight
    anchors.centerIn: parent
    ShapePath {
        fillGradient: RadialGradient {
            id: gradient
            centerX: width
            centerY: height
            centerRadius: width / 2
            focalX: centerX
            focalY: centerY
            GradientStop { position: 0; color: "blue" }
            GradientStop { position: 0.2; color: "green" }
            GradientStop { position: 0.4; color: "red" }
            GradientStop { position: 0.6; color: "yellow" }
            GradientStop { position: 1; color: "cyan" }
        }
        startX: 0; startY: 0;
        PathLine { x: width * 2; y: 0 }
        PathLine { x: width * 2; y: height * 2 }
        PathLine { x: 0; y: height * 2 }
    }
    Connections {
        target: webvfx
        function onRenderRequested(time) {
            gradient.centerX = (time * shape.width) + shape.width / 2;
	    }
    }
}