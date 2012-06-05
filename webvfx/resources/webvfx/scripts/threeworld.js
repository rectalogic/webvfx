// Copyright (c) 2011 Hewlett-Packard Development Company, L.P. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Build and fly through a 3D world using Three.js

var WebVfx = WebVfx || {};

WebVfx.ThreeWorld = function (width, height, nearPlane, farPlane, animationData, readyCallback, scene) {
    var self = this;

    this.tracker = new WebVfx.Tracker(function () {
        readyCallback();
        delete self.tracker;
    });

    var aspect = width / height;
    this.camera = new WebVfx.AnimatedCamera(aspect, nearPlane, farPlane,
                                            animationData);
    this.scene = scene ? scene : new THREE.Scene();
    this.scene.add(this.camera);

    this.widescreen = aspect > (4 / 3);

    // Create and size canvas ourself, more efficient to size it up front
    var canvas = document.createElement("canvas");
    canvas.width = width;
    canvas.height = height;
    this.renderer = new THREE.WebGLRenderer({ canvas: canvas,
                                              antialias: true });
    // Need to enable this for our shader
    if (!this.renderer.context.getExtension("OES_standard_derivatives"))
        console.log("OES_standard_derivatives not supported");
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
        if (!this.jsonLoader)
            this.jsonLoader = new THREE.JSONLoader();
        this.tracker.increment();
        this.jsonLoader.load(url, function (geometry) {
            var mesh = new THREE.Mesh(geometry, material);
            if (callback)
                callback(mesh);
            self.scene.add(mesh);
            self.tracker.decrement();
        });
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
    var texture = new THREE.Texture(image);
    WebVfx.trackTextureImage(texture, tracker);
    image.src = url;
    return texture;
};

WebVfx.trackTextureImage = function (texture, tracker) {
    var image = texture.image;
    if (!(image instanceof Image) || image.complete) {
        texture.needsUpdate = true;
        return;
    }
    tracker.increment();
    image.addEventListener("load", function () {
        texture.needsUpdate = true;
        tracker.decrement();
    });
};

///////////

// _options_ includes:
// fontStyle - e.g. "30px Helvetica". If it uses a font loaded with @font-face,
//   then you must use a WebVfx.FontTracker and create the text Texture after
//   the font has loaded.
// textColor - Text color css spec.
// backgroundColor - Background color css spec.
// textAlign - "left", "right" or "center".
// textBaseline - "top", "middle" or "bottom"
// margin - array of [left-right, top-bottom] margins
WebVfx.createTextTexture = function (text, targetWidth, targetHeight, options) {
    var fontStyle = options.fontStyle;
    var textColor = options.textColor;
    var backgroundColor = options.backgroundColor;
    var textAlign = options.textAlign;
    var textBaseline = options.textBaseline;

    // Canvas context provides measureText(), but this only exposes width.
    // So apply same font style to span and measure it.
    // FYI span text looks much better than canvas text because span uses
    // subpixel antialiasing
    var span = document.createElement("span");
    span.style.visibility = "hidden";
    span.style.font = fontStyle;
    span.style.whiteSpace = "nowrap";
    span.textContent = text;
    document.body.appendChild(span);
    var textWidth = span.offsetWidth;
    var textHeight = span.offsetHeight;
    document.body.removeChild(span);

    var boxWidth = targetWidth;
    var boxHeight = targetHeight;
    if (options.margin) {
        var margin = options.margin;
        boxWidth -= 2 * margin[0];
        boxHeight -= 2 * margin[1];
    }
    var scale = Math.min(boxWidth / textWidth, boxHeight / textHeight);

    var canvas = document.createElement("canvas");
    canvas.width = targetWidth;
    canvas.height = targetHeight;

    var context = canvas.getContext("2d");
    if (backgroundColor) {
        context.fillStyle = backgroundColor;
        context.fillRect(0, 0, targetWidth, targetHeight);
    }
    // Scale text to fit if too large
    var positionScale = 1;
    if (scale < 1) {
        context.scale(scale, scale);
        positionScale = 1 / scale;
    }
    context.font = fontStyle;
    context.fillStyle = textColor;

    var x = 0, y = 0;

    switch (textAlign) {
    case "right":
        x = targetWidth;
        break;
    case "center":
        x = targetWidth / 2;
        break;
    case "left":
    default:
        textAlign = "left";
        break;
    }
    context.textAlign = textAlign;

    switch (textBaseline) {
    case "middle":
        y = targetHeight / 2;
        break;
    case "bottom":
        y = targetHeight;
        break;
    case "top":
    default:
        textBaseline = "top";
        break;
    }
    context.textBaseline = textBaseline;

    context.fillText(text, x * positionScale, y * positionScale);

    var texture = new THREE.Texture(canvas);
    texture.needsUpdate = true;
    return texture;
}

///////////

WebVfx.updateVideoTexture = function (texture, name) {
    var image = webvfx.getImage(name);
    if (!image)
        return;
    texture.image = image.toImageData();
    texture.needsUpdate = true;
};

// v offset in uv space for top bottom to render 16:9 image in 4:3 mesh.
// 16:12 (4:3) vs 16:9, 12-9=3/2=1.5 so 1.5/9 is offset for top and bottom
WebVfx.vClampOffset = 1.5/9;

///////////

// Set UV coords for given layer, for a single quad
WebVfx.setQuadUvs = function (mesh, layer, u1, v1, u2, v2, u3, v3, u4, v4) {
    var uvs = [new THREE.UV(u1, v1),
               new THREE.UV(u2, v2),
               new THREE.UV(u3, v3),
               new THREE.UV(u4, v4)];
    mesh.geometry.faceVertexUvs[layer] = [uvs];
}

// Map (scale/translate) UVs from _fromLayer_ to _toLayer_
// min/max specifies new UV coordinate space range.
WebVfx.mapUvs = function (mesh, fromLayer, toLayer, minU, minV, maxU, maxV) {
    var uvs = mesh.geometry.faceVertexUvs[fromLayer];
    var length = uvs.length;

    // Find UV space we are mapping from
    var fromMinU = Number.MAX_VALUE;
    var fromMaxU = Number.MIN_VALUE;
    var fromMinV = Number.MAX_VALUE;
    var fromMaxV = Number.MIN_VALUE;
    for (var i = 0; i < length; i++) {
        var faceUv = uvs[i];
        var faceUvLength = faceUv.length;
        for (var j = 0; j < faceUvLength; j++) {
            var uv = faceUv[j];
            fromMinU = Math.min(uv.u, fromMinU);
            fromMaxU = Math.max(uv.u, fromMaxU);
            fromMinV = Math.min(uv.v, fromMinV);
            fromMaxV = Math.max(uv.v, fromMaxV);
        }
    }

    var fromDistU = fromMaxU - fromMinU;
    var fromDistV = fromMaxV - fromMinV;
    var toDistU = maxU - minU;
    var toDistV = maxV - minV;

    var mappedUvs = new Array(length);
    for (var i = 0; i < length; i++) {
        var faceUv = uvs[i];
        var faceUvLength = faceUv.length;
        var mappedFaceUv = new Array(faceUvLength);
        mappedUvs[i] = mappedFaceUv;
        for (var j = 0; j < faceUvLength; j++) {
            var uv = faceUv[j];
            var u = minU + (((uv.u - fromMinU) / fromDistU) * toDistU);
            var v = minV + (((uv.v - fromMinV) / fromDistV) * toDistV);
            mappedFaceUv[j] = new THREE.UV(u, v);
        }
    }

    mesh.geometry.faceVertexUvs[toLayer] = mappedUvs;
}

///////////

WebVfx.AnimatedCamera = function (aspect, nearPlane, farPlane, animationData) {
    var anim = new WebVfx.Animation(animationData);
    this.animation = anim;
    var fov = anim.radians2degrees(anim.verticalFOV(aspect));
    THREE.PerspectiveCamera.call(this, fov, aspect, nearPlane, farPlane);
    // Blender XYZ order is really ZYX
    this.eulerOrder = 'ZYX';
};

WebVfx.AnimatedCamera.prototype = new THREE.PerspectiveCamera();
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

