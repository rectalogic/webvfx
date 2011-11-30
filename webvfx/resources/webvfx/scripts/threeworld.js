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
    texture.image = webvfx.getImage(name).toImageData();
    texture.needsUpdate = true;
};

///////////

// Texture mesh by multiplying two textures.
// Texture regions outside of UV space can be colored.
// Parameters:
//  texture1 - THREE.Texture - uses main UV
//  texture2 - (optional) THREE.Texture - uses secondary UV
//  borderColor - (optional) THREE.Color - color of regions outside UV space.
//   Setting this also triggers antialiasing of the edges.
//  borderOpacity - float opacity of regions outside UV space (default 1.0)
//  lights - enable lighting if true
WebVfx.MultitextureShaderMaterial = function (parameters) {
    var vertexShader = this.shaderLibrary['vertexShader'];
    var fragmentShader = this.shaderLibrary['fragmentShader'];
    var vertexPrefix = [];
    var fragmentPrefix = [];

    var uniforms = THREE.UniformsUtils.merge([THREE.UniformsLib["lights"],
                                              THREE.UniformsLib["shadowmap"]]);
    uniforms.texture1 = { type: "t", value: 0, texture: parameters.texture1 };

    if (parameters.lights) {
        uniforms.ambient = { type: "c", value: new THREE.Color(0x050505) };
        vertexPrefix.push("#define USE_WEBVFX_LIGHTS");
        fragmentPrefix.push("#define USE_WEBVFX_LIGHTS");
    }

    if (parameters.texture2) {
        uniforms.texture2 = { type: "t", value: 1, texture: parameters.texture2 };
        vertexPrefix.push("#define USE_WEBVFX_TEXTURE2");
        fragmentPrefix.push("#define USE_WEBVFX_TEXTURE2");
    }
    if (parameters.borderColor) {
        uniforms.borderColor = { type: "c", value: parameters.borderColor };
        uniforms.borderOpacity = { type: "f", value: parameters.borderOpacity || 1.0 };
        fragmentPrefix.push("#define USE_WEBVFX_BORDERCOLOR");
    }

    if (vertexPrefix.length > 0)
        vertexShader = vertexPrefix.join("\n") + vertexShader;
    if (fragmentPrefix.length > 0)
        fragmentShader = fragmentPrefix.join("\n") + fragmentShader;

    THREE.ShaderMaterial.call(this, {
        uniforms: uniforms,
        fragmentShader: fragmentShader,
        vertexShader: vertexShader,
        lights: parameters.lights
    });
};

WebVfx.MultitextureShaderMaterial.prototype = new THREE.ShaderMaterial();
WebVfx.MultitextureShaderMaterial.prototype.constructor = WebVfx.MultitextureShaderMaterial;

// v offset in uv space for top bottom to render 16:9 image in 4:3 mesh.
// 16:12 (4:3) vs 16:9, 12-9=3/2=1.5 so 1.5/9 is offset for top and bottom
WebVfx.MultitextureShaderMaterial.vOffset = 1.5/9;

WebVfx.MultitextureShaderMaterial.prototype.shaderLibrary = {
    vertexShader: [
        "",
        "varying vec2 vUv;",
        "#ifdef USE_WEBVFX_TEXTURE2",
        "varying vec2 vUv2;",
        "#endif",
        "#ifdef USE_WEBVFX_LIGHTS",
        "varying vec3 vLightWeighting;",
        THREE.ShaderChunk["lights_lambert_pars_vertex"],
        THREE.ShaderChunk["shadowmap_pars_vertex"],
        "#endif",
        "void main() {",
        "    vUv = uv;",
        "#ifdef USE_WEBVFX_TEXTURE2",
        "    vUv2 = uv2;",
        "#endif",
        "    vec4 mvPosition = modelViewMatrix * vec4(position, 1.0);",
        "    vec3 transformedNormal = normalize(normalMatrix * normal);",
        "#ifdef USE_WEBVFX_LIGHTS",
             THREE.ShaderChunk["lights_lambert_vertex"],
        "#endif",
             THREE.ShaderChunk["default_vertex"],
        "#ifdef USE_WEBVFX_LIGHTS",
             THREE.ShaderChunk["shadowmap_vertex"],
        "#endif",
        "}"
    ].join("\n"),
    fragmentShader: [
        "",
        "varying vec2 vUv;",
        "uniform sampler2D texture1;",

        "#ifdef USE_WEBVFX_TEXTURE2",
        "varying vec2 vUv2;",
        "uniform sampler2D texture2;",
        "#endif",

        "#ifdef USE_WEBVFX_BORDERCOLOR",
        "#extension GL_OES_standard_derivatives : enable",
        "uniform vec3 borderColor;",
        "uniform float borderOpacity;",
        "#endif",

        "#ifdef USE_WEBVFX_LIGHTS",
        "varying vec3 vLightWeighting;",

        THREE.ShaderChunk["shadowmap_pars_fragment"],
        "#endif",

        "#ifdef USE_WEBVFX_BORDERCOLOR",
        "vec4 antialias(vec2 uv, vec2 fw, sampler2D texture, vec4 color) {",
        "    vec2 marginTL = -fw;",
        "    vec2 marginBR = 1.0 + fw;",
        //   // Completely outside the texture margins, use color
        "    if (uv.s <= marginTL.s || uv.s >= marginBR.s",
        "        || uv.t <= marginTL.t || uv.t >= marginBR.t)",
        "        return color;",
        //   // Completely inside the texture, use texture
        "    else if (uv.s >= 0.0 && uv.s <= 1.0 && uv.t >= 0.0 && uv.t <= 1.0)",
        "        return texture2D(texture, uv);",
        //   // Inside a margin - mix color and texture to antialias
        "    else {",
        //       // Amount of mix - 0.0 is all color, 1.0 is all texture
        "        float p = 0.0;",
        "        int c = 0;",
        "        if (uv.s < 0.0) {",
        "            p += smoothstep(marginTL.s, 0.0, uv.s);",
        "            c++;",
        "        }",
        "        else if (uv.s > 1.0) {",
        "            p += 1.0 - smoothstep(1.0, marginBR.s, uv.s);",
        "            c++;",
        "        }",
        "        if (uv.t < 0.0) {",
        "            p += smoothstep(marginTL.t, 0.0, uv.t);",
        "            c++;",
        "        }",
        "        else if (uv.t > 1.0) {",
        "            p += 1.0 - smoothstep(1.0, marginBR.t, uv.t);",
        "            c++;",
        "        }",
        "        return mix(color, texture2D(texture, uv), p / float(c));",
        "    }",
        "}",
        "#endif",

        "void main() {",
        "#ifdef USE_WEBVFX_LIGHTS",
        "    gl_FragColor = vec4(vLightWeighting, 1.0);",
        "#else",
        "    gl_FragColor = vec4(1.0);",
        "#endif",
        "#ifdef USE_WEBVFX_BORDERCOLOR",
        "    vec4 border = vec4(borderColor, borderOpacity);",
        "    gl_FragColor = gl_FragColor * antialias(vUv, fwidth(vUv), texture1, border);",
        "#else",
        "    gl_FragColor = gl_FragColor * texture2D(texture1, vUv);",
        "#endif",

        "#ifdef USE_WEBVFX_TEXTURE2",
        "#ifdef USE_WEBVFX_BORDERCOLOR",
        "    gl_FragColor = gl_FragColor * antialias(vUv2, fwidth(vUv2), texture2, border);",
        "#else",
        "    gl_FragColor = gl_FragColor * texture2D(texture2, vUv2);",
        "#endif",
        "#endif",
        "#ifdef USE_WEBVFX_LIGHTS",
            THREE.ShaderChunk["shadowmap_fragment"],
        "#endif",
        "}"
    ].join("\n")
};

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

