import QtQuick
import QtQuick3D
import QtQuick.Timeline
import org.webvfx.WebVfx 1.0

 View3D {
    id: view
    width: webvfx.videoWidth
    height: webvfx.videoHeight

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
                diffuseMap: VideoTexture {
                    imageName: "SourceImage"
                    imageType: webvfx.SourceImageType
                    flipV: true
                }
            }
        ]
    }

    Model {
        position: Qt.vector3d(200, 200, 0)
        source: "#Cube"
        scale.y: 9 / 16
        materials: [ DefaultMaterial {
                diffuseMap: VideoTexture {
                    imageName: "TargetImage"
                    imageType: webvfx.TargetImageType
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
                value: 200
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

    Connections {
        target: webvfx
        function onRenderRequested(time) {
            timeline.currentFrame = time * timeline.endFrame;
        }
    }
}
