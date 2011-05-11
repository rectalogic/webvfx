// Copyright (c) 2011 Hewlett-Packard Development Company, L.P. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

var WebVfx = WebVfx || {};


////////

// canvas - canvas HTML element
// shaderSource - fragment shader source code
// uniforms - hash mapping uniform name to value (float, array of float,
//  or Image/Canvas)
WebVfx.Shader = function (canvas, shaderSource, uniforms) {
    this.gl = canvas.getContext("experimental-webgl");

    this.uniforms = this.createUniforms(uniforms);
    this.program = this.compileProgram(shaderSource);
    this.buildQuad();
}

// Return the script text from a <script type="x-shader/x-fragment">
// element of the given ID.
WebVfx.Shader.loadShader = function (shaderId) {
    var shaderScript = document.getElementById(shaderId);
    if (!shaderScript || shaderScript.type != "x-shader/x-fragment") {
        throw "Could not find shader script ID " + shaderId +
                    " of type x-shader/x-fragment";
    }
    return shaderScript.text;
}

// Update an existing uniforms value
WebVfx.Shader.prototype.updateUniform = function (name, value) {
    this.use();
    this.uniforms[name].setValue(value);
}

// Rebuild uniforms with instances of our Uniform classes
WebVfx.Shader.prototype.createUniforms = function (uniforms) {
    var textureCount = 0;
    var newUniforms = {};
    for (var name in uniforms) {
        var uniform = uniforms[name];
        if (uniform instanceof HTMLImageElement ||
            uniform instanceof HTMLCanvasElement) {
            newUniforms[name] = new WebVfx.Texture(this.gl, textureCount++,
                                                   uniform);
        }
        else
            newUniforms[name] = new WebVfx.Uniform(this.gl, uniform);
    }
    return newUniforms;
}

WebVfx.Shader.prototype.use = function () {
    this.gl.useProgram(this.program);
}

WebVfx.Shader.VERTEX_SHADER_SOURCE = '\
    attribute vec2 vertex;\
    attribute vec2 _texCoord;\
    varying vec2 texCoord;\
    void main() {\
        texCoord = _texCoord;\
        gl_Position = vec4(vertex * 2.0 - 1.0, 0.0, 1.0);\
    }';

WebVfx.Shader.prototype.compileProgram = function (shaderSource) {
    var gl = this.gl;
    var program = gl.createProgram();

    var vs = this.compileSource(gl.VERTEX_SHADER,
                                WebVfx.Shader.VERTEX_SHADER_SOURCE);
    var fs = this.compileSource(gl.FRAGMENT_SHADER,
                                this.buildFragmentShaderSource(shaderSource));
    gl.attachShader(program, vs);
    gl.attachShader(program, fs);
    gl.linkProgram(program);
    if (!gl.getProgramParameter(program, gl.LINK_STATUS))
        throw 'link error: ' + gl.getProgramInfoLog(program);

    gl.deleteShader(vs);
    gl.deleteShader(fs);

    // Set uniform locations
    gl.useProgram(program);
    for (var name in this.uniforms) {
        var location = gl.getUniformLocation(program, name);
        if (location)
            this.uniforms[name].setLocation(location);
        else
            console.warn("unrecognized uniform name " + name);
    }

    return program;
}

// Prepend declarations to shader source
WebVfx.Shader.prototype.buildFragmentShaderSource = function (shaderSource) {
    var declarations = [ "#ifdef GL_ES",
                         "precision highp float;",
                         "#endif",
                         "varying vec2 texCoord;" ];
    for (var name in this.uniforms) {
        declarations.push(this.uniforms[name].getDeclaration(name));
    }
    declarations.push(shaderSource);
    return declarations.join("\n");
}

WebVfx.Shader.prototype.compileSource = function (type, shaderSource) {
    var gl = this.gl;
    var shader = gl.createShader(type);
    gl.shaderSource(shader, shaderSource);
    gl.compileShader(shader);
    if (!gl.getShaderParameter(shader, gl.COMPILE_STATUS))
        throw 'compile error: ' + gl.getShaderInfoLog(shader);
    return shader;
}

WebVfx.Shader.prototype.buildQuad = function () {
    var gl = this.gl;

    this.vertexBuffer = gl.createBuffer();
    gl.bindBuffer(gl.ARRAY_BUFFER, this.vertexBuffer);
    gl.bufferData(gl.ARRAY_BUFFER,
                  new Float32Array([ 0, 0, 0, 1, 1, 0, 1, 1 ]),
                  gl.STATIC_DRAW);

    this.texCoordBuffer = gl.createBuffer();
    gl.bindBuffer(gl.ARRAY_BUFFER, this.texCoordBuffer);
    gl.bufferData(gl.ARRAY_BUFFER,
                  new Float32Array([ 0, 0, 0, 1, 1, 0, 1, 1 ]),
                  gl.STATIC_DRAW);

    this.vertexAttribute = gl.getAttribLocation(this.program, 'vertex');
    gl.enableVertexAttribArray(this.vertexAttribute);
    this.texCoordAttribute = gl.getAttribLocation(this.program, '_texCoord');
    gl.enableVertexAttribArray(this.texCoordAttribute);
}

WebVfx.Shader.prototype.render = function () {
    var gl = this.gl;

    // Should use gl.drawingBufferWidth/Height but they aren't implemented
    gl.viewport(0, 0, gl.canvas.width, gl.canvas.height);

    this.use();
    gl.bindBuffer(gl.ARRAY_BUFFER, this.vertexBuffer);
    gl.vertexAttribPointer(this.vertexAttribute, 2, gl.FLOAT, false, 0, 0);
    gl.bindBuffer(gl.ARRAY_BUFFER, this.texCoordBuffer);
    gl.vertexAttribPointer(this.texCoordAttribute, 2, gl.FLOAT, false, 0, 0);
    gl.drawArrays(gl.TRIANGLE_STRIP, 0, 4);
}


////////

// value = initial value
WebVfx.Uniform = function (gl, value) {
    this.gl = gl;
    this.value = value;
    if (typeof(value) == "number") {
        this.uniformFunction = gl.uniform1f;
        this.declaration = "uniform float ";
    }
    else if (value instanceof Array) {
        this.wrapperConstructor = Float32Array;
        switch (value.length) {
        case 1:
            this.uniformFunction = gl.uniform1fv;
            this.declaration = "uniform vec1 ";
            break;
        case 2:
            this.uniformFunction = gl.uniform2fv;
            this.declaration = "uniform vec2 ";
            break;
        case 3:
            this.uniformFunction = gl.uniform3fv;
            this.declaration = "uniform vec3 ";
            break;
        case 4:
            this.uniformFunction = gl.uniform4fv;
            this.declaration = "uniform vec4 ";
            break;
        default:
            throw 'invalid uniform value array length: ' + value;
        }
    }
    else
        throw 'invalid uniform value: ' + value;
}

WebVfx.Uniform.prototype.setLocation = function(uniformLocation) {
    this.uniformLocation = uniformLocation;
    this.setValue(this.value);
}

WebVfx.Uniform.prototype.getDeclaration = function(name) {
    return this.declaration + name + ";";
}

// Set uniform value, can be float or array of floats
WebVfx.Uniform.prototype.setValue = function(value) {
    this.value = value;
    if (this.wrapperConstructor)
        value = new this.wrapperConstructor(value);
    this.uniformFunction.call(this.gl, this.uniformLocation, value);
}

////////

// unit - texture unit, 0-7
// image - initial image
WebVfx.Texture = function (gl, unit, image) {
    this.gl = gl;
    this.unit = unit;
    this.id = gl.createTexture();

    gl.activeTexture(gl.TEXTURE0 + this.unit);
    gl.bindTexture(gl.TEXTURE_2D, this.id);
    gl.texParameteri(gl.TEXTURE_2D, gl.TEXTURE_MAG_FILTER, gl.LINEAR);
    gl.texParameteri(gl.TEXTURE_2D, gl.TEXTURE_MIN_FILTER, gl.LINEAR);
    gl.texParameteri(gl.TEXTURE_2D, gl.TEXTURE_WRAP_S, gl.CLAMP_TO_EDGE);
    gl.texParameteri(gl.TEXTURE_2D, gl.TEXTURE_WRAP_T, gl.CLAMP_TO_EDGE);

    this.image = image;
}

WebVfx.Texture.prototype.setLocation = function(uniformLocation) {
    var gl = this.gl;
    gl.uniform1i(uniformLocation, this.unit);
    this.setValue(this.image);
}

WebVfx.Texture.prototype.getDeclaration = function(name) {
    return "uniform sampler2D " + name + ";";
}

// Upload image into texture
WebVfx.Texture.prototype.setValue = function(image) {
    var gl = this.gl;
    gl.bindTexture(gl.TEXTURE_2D, this.id);
    // Flip texture vertically so it's not upside down
    gl.pixelStorei(gl.UNPACK_FLIP_Y_WEBGL, true);
    gl.texImage2D(gl.TEXTURE_2D, 0, gl.RGBA, gl.RGBA, gl.UNSIGNED_BYTE, image);
}
