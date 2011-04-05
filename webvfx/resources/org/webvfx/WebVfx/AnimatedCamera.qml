// Copyright (c) 2011 Hewlett-Packard Development Company, L.P. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

import Qt 4.7
import Qt3D 1.0

// Qt3D Camera that supports animation data exported from Blender

import "qrc:/script/webvfx-camera.js" as WebVfx

Camera {

    function computeFOV(width, height) {
        if (!WebVfx.CameraAnimation.prototype.global)
            return 60;
        return WebVfx.radians2degrees(WebVfx.CameraAnimation.prototype.global.verticalFOV(width, height));
    }

    function updateCamera() {
        var animation = WebVfx.CameraAnimation.prototype.global;
        animation.evaluate(animationTime);
        eye = Qt.vector3d(animation.eye[0],
                          animation.eye[1],
                          animation.eye[2]);
        center = Qt.vector3d(animation.lookAt[0],
                             animation.lookAt[1],
                             animation.lookAt[2]);
        upVector = Qt.vector3d(animation.upVector[0],
                               animation.upVector[1],
                               animation.upVector[2]);
    }

    // Animation data JSON
    property variant animationData
    onAnimationDataChanged: {
        // Stash a global instance, can't store a JS object in QML
        WebVfx.CameraAnimation.prototype.global = new WebVfx.CameraAnimation(animationData)
        // Update FOV
        fieldOfView = computeFOV(width, height);
        updateCamera();
    }

    // Animation time (0..1)
    property real animationTime: 0
    onAnimationTimeChanged: updateCamera()

    fieldOfView: computeFOV(width, height)

    property Connections connections : Connections {
        target: webvfx
        onRenderRequested: camera.animationTime = time
    }
}
