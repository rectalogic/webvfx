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

// _options_ includes:
// fontStyle - e.g. "30px Helvetica". If it uses a font loaded with @font-face,
//   then you must use a WebVfx.FontTracker and create the text Texture after
//   the font has loaded.
// textColor - Text color css spec.
// backgroundColor - Background color css spec.
// textAlign - "left", "right" or "center".
// textBaseline - "top", "middle" or "bottom"
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
    texture.image = webvfx.getImage(name).toImageData();
    texture.needsUpdate = true;
};

///////////

// Texture mesh by multiplying two textures.
// Texture regions outside of UV space can be colored.
// Parameters:
//  texture1 - THREE.Texture - uses main UV
//  texture2 - (optional) THREE.Texture - uses secondary UV
//  borderColor - (optional) THREE.Color - color of regions outside UV space
//  borderOpacity - float opacity of regions outside UV space (default 1.0)
WebVfx.MeshMultitextureMaterial = function (parameters) {
    var vertexShader = this.shaderLibrary['vertexShader'];
    var fragmentShader = this.shaderLibrary['fragmentShader'];
    var vertexPrefix = [];
    var fragmentPrefix = [];

    var uniforms = {
        texture1: { type: "t", value: 0, texture: parameters.texture1 }
    };
    if (parameters.texture2) {
        uniforms.texture2 = { type: "t", value: 1, texture: parameters.texture2 };
        vertexPrefix.push("#define USE_TEXTURE2");
        fragmentPrefix.push("#define USE_TEXTURE2");
    }
    if (parameters.borderColor) {
        uniforms.borderColor = { type: "c", value: parameters.borderColor };
        uniforms.borderOpacity = { type: "f", value: parameters.borderOpacity || 1.0 };
        fragmentPrefix.push("#define USE_BORDERCOLOR");
    }

    if (vertexPrefix.length > 0)
        vertexShader = vertexPrefix.join("\n") + vertexShader;
    if (fragmentPrefix.length > 0)
        fragmentShader = fragmentPrefix.join("\n") + fragmentShader;

    THREE.MeshShaderMaterial.call(this, {
        uniforms: uniforms,
        fragmentShader: fragmentShader,
        vertexShader: vertexShader
    });
};

WebVfx.MeshMultitextureMaterial.prototype = new THREE.MeshShaderMaterial();
WebVfx.MeshMultitextureMaterial.prototype.constructor = WebVfx.MeshMultitextureMaterial;

WebVfx.MeshMultitextureMaterial.prototype.shaderLibrary = {
    vertexShader: [
        "",
        "varying vec2 vUv;",
        "#ifdef USE_TEXTURE2",
        "varying vec2 vUv2;",
        "#endif",
        "void main() {",
        "    vUv = uv;",
        "#ifdef USE_TEXTURE2",
        "    vUv2 = uv2;",
        "#endif",
        "    gl_Position = projectionMatrix * modelViewMatrix * vec4(position, 1.0);",
        "}"
    ].join("\n"),
    fragmentShader: [
        "",
        "varying vec2 vUv;",
        "uniform sampler2D texture1;",
        "#ifdef USE_TEXTURE2",
        "varying vec2 vUv2;",
        "uniform sampler2D texture2;",
        "#endif",
        "#ifdef USE_BORDERCOLOR",
        "uniform vec3 borderColor;",
        "uniform float borderOpacity;",
        "#endif",
        "void main() {",
        "    vec4 texture1Color;",
        "#ifdef USE_BORDERCOLOR",
        "    vec4 border = vec4(borderColor, borderOpacity);",
        "    if (vUv.s < 0.0 || vUv.s > 1.0 || vUv.t < 0.0 || vUv.t > 1.0)",
        "        texture1Color = border;",
        "    else",
        "#endif",
        "        texture1Color = texture2D(texture1, vUv);",
        "    gl_FragColor = texture1Color;",
        "#ifdef USE_TEXTURE2",
        "    vec4 texture2Color;",
        "#ifdef USE_BORDERCOLOR",
        "    if (vUv2.s < 0.0 || vUv2.s > 1.0 || vUv2.t < 0.0 || vUv2.t > 1.0)",
        "        texture2Color = border;",
        "    else",
        "#endif",
        "        texture2Color = texture2D(texture2, vUv2);",
        "    gl_FragColor = gl_FragColor * texture2Color;",
        "#endif",
        "}"
    ].join("\n")
};

///////////

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

