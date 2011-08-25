// Copyright (c) 2011 Hewlett-Packard Development Company, L.P. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Build and fly through a 3D world using Three.js

var WebVfx = WebVfx || {};

WebVfx.ThreeWorld = function (width, height, nearPlane, farPlane, animationData, readyCallback) {
    var self = this;

    this.tracker = new WebVfx.Tracker(function () {
        readyCallback();
        delete self.tracker;
    });
    this.jsonLoader = new THREE.JSONLoader();

    var aspect = width / height;
    this.camera = new WebVfx.AnimatedCamera(aspect, nearPlane, farPlane,
                                            animationData);
    this.scene = new THREE.Scene();
    // Blender OBJ and three.js exporters rotate scene by -PI/2 in X,
    // undo that here so our camera animation data works.
    this.scene.rotation.x = Math.PI / 2;
    this.scene.updateMatrix();

    this.widescreen = aspect > (4 / 3);

    // Create and size canvas ourself, more efficient to size it up front
    var canvas = document.createElement("canvas");
    canvas.width = width;
    canvas.height = height;
    this.renderer = new THREE.WebGLRenderer({ canvas: canvas,
                                              antialias: true });
    this.renderer.setViewport(0, 0, width, height);
    document.body.appendChild(canvas);

    // Treat page load as trackable
    if (document.readyState != "complete") {
        this.tracker.increment();
        window.addEventListener('load', function (event) {
            self.tracker.decrement();
        }, false);
    }
};

WebVfx.ThreeWorld.prototype = {
    // Must be called to start resource tracking
    startTracking: function () {
        this.tracker.start();
    },

    // Optional _callback_ will be passed the Mesh for additional configuring
    addJSONModel: function (url, material, callback) {
        var self = this;
        this.tracker.increment();
        this.jsonLoader.load({ model: url, callback: function (geometry) {
            var mesh = new THREE.Mesh(geometry, material);
            if (callback)
                callback(mesh);
            self.scene.addObject(mesh);
            self.tracker.decrement();
        } });
    },

    resize: function (width, height) {
        this.renderer.setSize(width, height);
        this.camera.aspect = width / height;
        this.camera.updateProjectionMatrix();
        this.widescreen = this.camera.aspect > (4 / 3);
    },

    render: function (time) {
        this.camera.setAnimationTime(time);
        this.renderer.render(this.scene, this.camera);
    }
};

///////////

WebVfx.loadImageTexture = function (url, tracker) {
    var image = new Image();
    tracker.increment();
    var texture = new THREE.Texture(image);
    image.onload = function () {
        texture.needsUpdate = true;
        tracker.decrement();
    };
    image.src = url;
    return texture;
};

///////////

// If _fontStyle_ uses a font loaded with @font-face, then you must
// use a WebVfx.FontTracker and create the text Texture after the font
// has loaded.
WebVfx.createTextTexture = function (text, fontStyle, color, targetWidth, targetHeight) {
    // Canvas context provides measureText(), but this only exposes width.
    // So apply same font style to span and measure it.
    // FYI span text looks much better than canvas text because span uses
    // subpixel antialiasing
    var span = document.createElement("span");
    span.style.visibility = "hidden";
    span.style.font = fontStyle;
    span.textContent = text;
    document.body.appendChild(span);
    var textWidth = span.offsetWidth;
    var textHeight = span.offsetHeight;
    document.body.removeChild(span);

    var scale = Math.min(targetWidth / textWidth, targetHeight / textHeight);

    var canvas = document.createElement("canvas");
    canvas.width = targetWidth;
    canvas.height = targetHeight;

    var context = canvas.getContext("2d");
    // Scale text to fit if too large
    if (scale < 1)
        context.scale(scale, scale);
    context.font = fontStyle;
    context.fillStyle = color;
    context.textAlign = "left";
    context.textBaseline = "top";
    context.fillText(text, 0, 0);

    var texture = new THREE.Texture(canvas);
    texture.needsUpdate = true;
    return texture;
}

///////////

WebVfx.updateVideoTexture = function (texture, name) {
    texture.image = webvfx.getImage(name).toImageData();
    texture.needsUpdate = true;
};

///////////

WebVfx.createTexturedMaterial = function (texture, transparent, lightMap) {
    return new THREE.MeshBasicMaterial({ map: texture,
                                         transparent: transparent,
                                         lightMap: lightMap });
};

WebVfx.addVideoQuadUvs = function (mesh, u1, v1, u2, v2, u3, v3, u4, v4) {
    var uvs = [new THREE.UV(u1, v1),
               new THREE.UV(u2, v2),
               new THREE.UV(u3, v3),
               new THREE.UV(u4, v4)];
    mesh.geometry.faceVertexUvs.push([uvs]);
}

///////////

WebVfx.AnimatedCamera = function (aspect, nearPlane, farPlane, animationData) {
    var anim = new WebVfx.Animation(animationData);
    this.animation = anim;
    var fov = anim.radians2degrees(anim.verticalFOV(aspect));
    THREE.Camera.call(this, fov, aspect, nearPlane, farPlane);
    this.useTarget = false;
    // Blender XYZ order is really ZYX
    this.eulerOrder = 'ZYX';
};

WebVfx.AnimatedCamera.prototype = new THREE.Camera();
WebVfx.AnimatedCamera.prototype.constructor = WebVfx.AnimatedCamera;

WebVfx.AnimatedCamera.prototype.setAnimationTime = function (time) {
    var anim = this.animation;
    anim.evaluateTime(time);
    this.position.x = anim.locationX;
    this.position.y = anim.locationY;
    this.position.z = anim.locationZ;
    this.rotation.x = anim.rotationX;
    this.rotation.y = anim.rotationY;
    this.rotation.z = anim.rotationZ;
};

