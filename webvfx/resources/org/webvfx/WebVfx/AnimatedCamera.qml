// Copyright (c) 2011 Hewlett-Packard Development Company, L.P. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

import Qt 4.7
import Qt3D 1.0

// QtQuick3D Camera that supports animation data exported from Blender

import "qrc:/webvfx/script/animation.js" as Anim

Camera {

    function computeFOV(animationConstructed, width, height) {
        var animation = getAnimation();
        if (!animation)
            return 0;
        return animation.radians2degrees(animation.verticalFOV(width / height));
    }

    function updateCamera() {
        var animation = getAnimation();
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

    function getAnimation() {
        return Anim.WebVfx.Animation.prototype.global;
    }

    // We can't bind to a JS native property, so this is a pseudo-property
    // that changes whenever the native JS Animation instance is changed.
    property int animationConstructed: 0

    // Animation data JSON
    property variant animationData
    onAnimationDataChanged: {
        // QML doesn't allow globals, and storing a JS object as a variant
        // property doesn't work - so stash it on the prototype.
        Anim.WebVfx.Animation.prototype.global = new Anim.WebVfx.Animation(animationData);
        // Tweak property so data binding fires
        animationConstructed += 1;
        updateCamera();
    }


    // Animation time (0..1)
    property real animationTime: 0
    onAnimationTimeChanged: updateCamera()

    // If true, then reverse the sense of time
    property bool reverseTime: false

    // Need to make this a property because Camera doesn't
    // support arbitrary children.
    // Also this allows the user to override us.
    property Connections connections : Connections {
        target: webvfx
        onRenderRequested: animationTime = reverseTime ? 1 - time : time
    }

    // Recompute FOV if viewport size or Animation change.
    // animationConstructed is a pseudo-property used to detect when
    // an Animation has been constructed.
    fieldOfView: computeFOV(animationConstructed, width, height)
}
