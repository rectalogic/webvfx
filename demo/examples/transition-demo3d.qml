import QtQuick
import QtQuick3D
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
        position: Qt.vector3d(0, 200, 300)
        eulerRotation.x: -30
    }

    DirectionalLight {
        eulerRotation.x: -30
        eulerRotation.y: -70
    }

    Model {
        position: Qt.vector3d(0, -200, 0)
        source: "#Cube"
        materials: [ DefaultMaterial {
                diffuseMap: VideoTexture {
                    imageName: "SourceImage"
                    imageType: webvfx.SourceImageType
                }
            }
        ]
    }

    Model {
        position: Qt.vector3d(0, 150, 0)
        source: "#Sphere"
        materials: [ DefaultMaterial {
                diffuseMap: VideoTexture {
                    imageName: "TargetImage"
                    imageType: webvfx.TargetImageType
                }
            }
        ]
    }
}
