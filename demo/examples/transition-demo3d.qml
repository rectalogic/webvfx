import QtQuick
import QtQuick3D
import QtQuick.Timeline
import QtMultimedia
import stream.webvfx.WebVfx

 View3D {
    id: view
    width: webvfx.videoSize.width
    height: webvfx.videoSize.height

    environment: SceneEnvironment {
        clearColor: "skyblue"
        backgroundMode: SceneEnvironment.Color
        antialiasingMode: SceneEnvironment.MSAA
        antialiasingQuality: SceneEnvironment.High
    }

    PerspectiveCamera {
        id: camera
    }

    DirectionalLight {
        eulerRotation.x: -30
        eulerRotation.y: -70
    }

    Model {
        position: Qt.vector3d(0, 0, 0)
        source: "#Cube"
        scale.y: 9 / 16
        materials: [ DefaultMaterial {
                diffuseMap: Texture {
                    textureData: VideoTextureData {
                        id: sourceTexture
                    }
                    flipV: true
                }
            }
        ]
    }

    Node {
        position: Qt.vector3d(100, 100, 0)
        Rectangle {
            anchors.horizontalCenter: parent.horizontalCenter
            color: "blue"
            width: text3d.width + 20
            height: text3d.height + 20
            Text {
                id: text3d
                anchors.horizontalCenter: parent.horizontalCenter
                anchors.verticalCenter: parent.verticalCenter
                text: webvfx.getStringParameter("title")
                font.pointSize: 32
                color: "white"
            }
        }
    }

    Model {
        position: Qt.vector3d(200, 200, 0)
        source: "#Cube"
        scale.y: 9 / 16
        materials: [ DefaultMaterial {
                diffuseMap: Texture {
                    textureData: VideoTextureData {
                        id: targetTexture
                    }
                    flipV: true
                }
            }
        ]
    }

    Timeline {
        id: timeline
        startFrame: 0
        endFrame: 1000
        enabled: true

        KeyframeGroup {
            target: camera
            property: "position.x"

            Keyframe {
                frame: 0
                value: 0
            }
            Keyframe {
                frame: 1000
                value: 101.5
            }
        }
        KeyframeGroup {
            target: camera
            property: "position.y"

            Keyframe {
                frame: 0
                value: 0
            }
            Keyframe {
                frame: 1000
                value: 200
            }
        }
        KeyframeGroup {
            target: camera
            property: "position.z"

            Keyframe {
                frame: 0
                value: 98
            }
            Keyframe {
                frame: 500
                value: 250
            }
            Keyframe {
                frame: 1000
                value: 0
            }
        }
        KeyframeGroup {
            target: camera
            property: "eulerRotation.y"

            Keyframe {
                frame: 0
                value: 0
            }
            Keyframe {
                frame: 1000
                value: -90
            }
        }
    }

    Component.onCompleted: {
        webvfx.appendVideoSink(webvfx.addVideoSource(), sourceTexture.videoSink);
        webvfx.appendVideoSink(webvfx.addVideoSource(), targetTexture.videoSink);
    }

    Connections {
        target: webvfx
        function onRenderRequested(time) {
            timeline.currentFrame = time * timeline.endFrame;
        }
    }
}
