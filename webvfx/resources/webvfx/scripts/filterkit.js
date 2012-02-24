// Copyright (c) 2012 Hewlett-Packard Development Company, L.P. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Requires shaderkit.js

var FilterKit = FilterKit || {};

////////

FilterKit.subclass = function (constructor, superConstructor) {
    constructor.prototype = new superConstructor();
    constructor.prototype.constructor = constructor;
}

FilterKit.TEXTURE_TOP = "topTex";
FilterKit.TEXTURE_BOTTOM = "bottomTex";

////////

FilterKit.FilterTree = function () {
}

//// Filters

FilterKit.FilterTree.prototype.adjust = function (r, g, b) {
    this.rootFilter = new FilterKit.Filter.Adjust(r, g, b, this.rootFilter);
    return this;
}

FilterKit.FilterTree.prototype.bias = function (amount) {
    this.rootFilter = new FilterKit.Filter.Bias(amount, this.rootFilter);
    return this;
}

// factor - amount of color brightness
FilterKit.FilterTree.prototype.brightness = function (factor) {
    this.rootFilter = new FilterKit.Filter.Brightness(factor, this.rootFilter);
    return this;
}

FilterKit.FilterTree.prototype.contrast = function (factor) {
    this.rootFilter = new FilterKit.Filter.Contrast(factor, this.rootFilter);
    return this;
}

// darkColor - [R, G, B] array (values from 0->1)
// lightColor - [R, G, B] array
FilterKit.FilterTree.prototype.duoTone = function (darkColor, lightColor) {
    this.rootFilter = new FilterKit.Filter.DuoTone(darkColor, lightColor, this.rootFilter);
    return this;
}

FilterKit.FilterTree.prototype.fill = function (r, g, b, a) {
    this.rootFilter = new FilterKit.Filter.Fill(r, g, b, a, this.rootFilter);
    return this;
}

FilterKit.FilterTree.prototype.gamma = function (gamma) {
    this.rootFilter = new FilterKit.Filter.Gamma(gamma, this.rootFilter);
    return this;
}

FilterKit.FilterTree.prototype.invert = function () {
    this.rootFilter = new FilterKit.Filter.Invert(this.rootFilter);
    return this;
}

FilterKit.FilterTree.prototype.opacity = function (opacity) {
    this.rootFilter = new FilterKit.Filter.Opacity(opacity, this.rootFilter);
    return this;
}

// levels - number of color levels
FilterKit.FilterTree.prototype.posterize = function (levels) {
    this.rootFilter = new FilterKit.Filter.Posterize(levels, this.rootFilter);
    return this;
}

FilterKit.FilterTree.prototype.rescale = function (darkColor, lightColor) {
    this.rootFilter = new FilterKit.Filter.Rescale(darkColor, lightColor, this.rootFilter);
    return this;
}

// factor - amount of color saturation, 0 is no color, 1 is normal color, >1 is saturated
FilterKit.FilterTree.prototype.saturation = function (factor) {
    this.rootFilter = new FilterKit.Filter.Saturation(factor, this.rootFilter);
    return this;
}

FilterKit.FilterTree.prototype.sepia = function () {
    this.rootFilter = new FilterKit.Filter.Sepia(this.rootFilter);
    return this;
}

// inner - radius of inner edge
// outer - radious of outer edge
// color - optional vignette color [R, G, B] (default is black)
FilterKit.FilterTree.prototype.vignette = function (inner, outer, color) {
    this.rootFilter = new FilterKit.Filter.Vignette(inner, outer, color, this.rootFilter);
    return this;
}

//// Blends

FilterKit.FilterTree.prototype.addition = function (topFilterTree) {
    this.rootFilter = new FilterKit.Filter.Addition(topFilterTree.rootFilter, this.rootFilter);
    return this;
}

FilterKit.FilterTree.prototype.difference = function (topFilterTree) {
    this.rootFilter = new FilterKit.Filter.Difference(topFilterTree.rootFilter, this.rootFilter);
    return this;
}

FilterKit.FilterTree.prototype.multiply = function (topFilterTree) {
    this.rootFilter = new FilterKit.Filter.Multiply(topFilterTree.rootFilter, this.rootFilter);
    return this;
}

FilterKit.FilterTree.prototype.normal = function (topFilterTree) {
    this.rootFilter = new FilterKit.Filter.Normal(topFilterTree.rootFilter, this.rootFilter);
    return this;
}

FilterKit.FilterTree.prototype.overlay = function (topFilterTree) {
    this.rootFilter = new FilterKit.Filter.Overlay(topFilterTree.rootFilter, this.rootFilter);
    return this;
}

FilterKit.FilterTree.prototype.screen = function (topFilterTree) {
    this.rootFilter = new FilterKit.Filter.Screen(topFilterTree.rootFilter, this.rootFilter);
    return this;
}

FilterKit.FilterTree.prototype.softLight = function (topFilterTree) {
    this.rootFilter = new FilterKit.Filter.SoftLight(topFilterTree.rootFilter, this.rootFilter);
    return this;
}

//// Convolves

FilterKit.FilterTree.prototype.blur = function () {
    this.rootFilter = new FilterKit.Filter.Blur(this.rootFilter);
    return this;
}

FilterKit.FilterTree.prototype.boxBlur = function () {
    this.rootFilter = new FilterKit.Filter.BoxBlur(this.rootFilter);
    return this;
}

FilterKit.FilterTree.prototype.bump = function () {
    this.rootFilter = new FilterKit.Filter.Bump(this.rootFilter);
    return this;
}

FilterKit.FilterTree.prototype.emboss = function () {
    this.rootFilter = new FilterKit.Filter.Emboss(this.rootFilter);
    return this;
}

FilterKit.FilterTree.prototype.gaussianBlur = function () {
    this.rootFilter = new FilterKit.Filter.GaussianBlur(this.rootFilter);
    return this;
}

FilterKit.FilterTree.prototype.sobelEdge = function () {
    this.rootFilter = new FilterKit.Filter.SobelEdge(this.rootFilter);
    return this;
}

FilterKit.FilterTree.prototype.sharpen = function () {
    this.rootFilter = new FilterKit.Filter.Sharpen(this.rootFilter);
    return this;
}

FilterKit.FilterTree.prototype.superSharpen = function () {
    this.rootFilter = new FilterKit.Filter.SuperSharpen(this.rootFilter);
    return this;
}

////////

// canvas - HTML Canvas to render into
// filterTree - FilterKit.FilterTree with the filters to apply when rendering
FilterKit.ShaderTree = function (canvas, filterTree) {
    this.renderer = new ShaderKit.Renderer(canvas);
    this.filters = [];
    var shaderData = this._buildShaderTreeRecursive(filterTree.rootFilter, {});
    this.rootShader = shaderData.shader;

    delete this.filters;
    delete this.topShader;
    delete this.bottomShader;

    this.renderTargetPool = [];
}

// image - the image to render through the shader tree
FilterKit.ShaderTree.prototype.render = function (image) {
    if (!this.sourceTexture)
        this.sourceTexture = new ShaderKit.SharedTexture(this.renderer);
    this.sourceTexture.setImage(image);
    this._renderRecursive(this.rootShader, false);
}

FilterKit.ShaderTree.prototype._renderRecursive = function (shader, renderToTarget) {
    var topRender, bottomRender;

    if (shader.topShader)
        topRender = this._renderRecursive(shader.topShader, true);
    else
        topRender = this.sourceTexture;

    if (shader.bottomShader)
        bottomRender = this._renderRecursive(shader.bottomShader, true);
    else
        bottomRender = this.sourceTexture;

    //XXX ugh, bottomTex uniform can be optimized out so it's not in the program
    shader.updateUniform(FilterKit.TEXTURE_TOP, topRender);
    shader.updateUniform(FilterKit.TEXTURE_BOTTOM, bottomRender);

    var shaderRender;
    if (renderToTarget)
        shaderRender = this._allocateRenderTarget();

    this.renderer.render(shader, shaderRender);

    this._releaseRenderTarget(topRender);
    this._releaseRenderTarget(bottomRender);

    return shaderRender;
}

FilterKit.ShaderTree.prototype._allocateRenderTarget = function () {
    var renderTarget = this.renderTargetPool.pop();
    if (!renderTarget)
        renderTarget = new ShaderKit.RenderTarget(this.renderer);
    return renderTarget;
}

FilterKit.ShaderTree.prototype._releaseRenderTarget = function (renderTarget) {
    // Don't want to push images or null
    if (renderTarget instanceof ShaderKit.RenderTarget)
        this.renderTargetPool.push(renderTarget);
}

FilterKit.ShaderTree.prototype._buildShaderTreeRecursive = function (filter, shaderData) {
    var topShaderData, bottomShaderData;

    if (filter.topFilter) {
        var data = { expression: shaderData.expression,
                     texture: FilterKit.TEXTURE_TOP };
        topShaderData = this._buildShaderTreeRecursive(filter.topFilter, data);
        if (topShaderData.shader)
            this.topShader = topShaderData.shader;
    }
    else
        topShaderData = { texture: FilterKit.TEXTURE_TOP };

    if (filter.bottomFilter) {
        var data = { expression: shaderData.expression,
                     texture: FilterKit.TEXTURE_BOTTOM };
        bottomShaderData = this._buildShaderTreeRecursive(filter.bottomFilter, data);
        if (bottomShaderData.shader)
            this.bottomShader = bottomShaderData.shader;
    }
    else
        bottomShaderData = { texture: FilterKit.TEXTURE_BOTTOM };

    // Convolve is special. Build a Shader for what we have so far,
    // and it will be rendered to a target and input to the convolve.
    if (filter instanceof FilterKit.Filter.Convolve) {
        if (topShaderData.expression) {
            var shader = this._buildShader(topShaderData.expression);
            shader.topShader = this.topShader;
            shader.bottomShader = this.bottomShader;
            shaderData.shader = shader;
        }

        // Convolve only uses the top expression, set it to the texture
        // we need to read from. A Convolve may need to read from the bottom
        // texture if it is the bottom of a Blend.
        topShaderData.expression = shaderData.texture ? shaderData.texture : FilterKit.TEXTURE_TOP;
    }
    // Not a Convolve, set expressions if unset
    else {
        if (!topShaderData.expression)
            topShaderData.expression = "texture2D(" + topShaderData.texture + ", texCoord)";
        if (!bottomShaderData.expression)
            bottomShaderData.expression = "texture2D(" + bottomShaderData.texture + ", texCoord)";
    }

    // Store filters
    this.filters.push(filter);

    shaderData.expression = filter.expression(topShaderData.expression, bottomShaderData.expression);

    // Root node
    if (!shaderData.texture) {
        var shader = this._buildShader(shaderData.expression);
        // If shaderData.shader is set, then the root node is a convolve
        // and so we need to use it as convolves topShader
        shader.topShader = shaderData.shader ? shaderData.shader : this.topShader;
        shader.bottomShader = this.bottomShader;
        shaderData.shader = shader;
    }

    return shaderData;
}

FilterKit.ShaderTree.SHADER_PREFIX = [
    ""
    ,"precision mediump float;"
    ,"varying vec2 texCoord;"
    ,"uniform sampler2D " + FilterKit.TEXTURE_TOP + ";"
    ,"uniform sampler2D " + FilterKit.TEXTURE_BOTTOM + ";"
    ,""
].join("\n"),

FilterKit.ShaderTree.prototype._buildShader = function (expression) {
    var shaderUniforms = {};
    var shaderExtensionNames = {};
    var shaderExtensionSource = [];
    var shaderLibraryNames = {};
    var shaderLibrarySource = [];
    for (var i = 0; i < this.filters.length; i++) {
        var filter = this.filters[i];

        // Get set of unique libraries
        var libraryNames = filter.libraryNames;
        if (libraryNames) {
            for (var n = 0; n < libraryNames.length; n++) {
                var name = libraryNames[n];
                if (!shaderLibraryNames[name]) {
                    shaderLibrarySource.push(FilterKit.Filter.library[name]);
                    shaderLibraryNames[name] = true;
                }
            }
        }

        var uniforms = filter.uniformValues;
        if (uniforms) {
            for (var u in uniforms) {
                if (uniforms.hasOwnProperty(u))
                    shaderUniforms[u] = uniforms[u];
            }
        }

        var extensionNames = filter.extensionNames;
        if (extensionNames) {
            for (var gle in extensionNames) {
                if (extensionNames.hasOwnProperty(gle)) {
                    if (!shaderExtensionNames[gle]) {
                        shaderExtensionSource.push("#extension " + gle + " : enable");
                        var extension = extensionNames[gle];
                        if (!this.renderer.enableExtension(extension))
                            console.warn(extension + " not supported");
                        shaderExtensionNames[gle] = true;
                    }
                }
            }
        }
    }
    // Reset
    this.filters = [];

    var shaderSource =
        shaderExtensionSource.join("\n") +
        FilterKit.ShaderTree.SHADER_PREFIX +
        shaderLibrarySource.join("\n") +
        [
            ""
            ,"void main() {"
            ,    "gl_FragColor = " + expression + ";"
            ,"}"
        ].join("\n")

    return new ShaderKit.Shader(this.renderer, shaderSource, shaderUniforms);
}

////////

FilterKit.Filter = function (topFilter) {
    this.topFilter = topFilter;
}

// Subclasses can store libraries here
FilterKit.Filter.library = {};

FilterKit.Filter.library.Clamp = [
    ,"vec4 clamp(vec4 color) {"
    ,    "return clamp(color, 0.0, 1.0);"
    ,"}"
    ,"vec3 clamp(vec3 color) {"
    ,    "return clamp(color, 0.0, 1.0);"
    ,"}"
].join("\n");

// Luma coefficients for linear RGB.
// BT.709 and sRGB would be (0.2126, 0.7152, 0.0722)
// Rec.601 (0.299, 0.587, 0.114)
FilterKit.Filter.library.Luma = "const vec4 lumaCoeff = vec4(0.3086, 0.6094, 0.0820, 1.0);";

FilterKit.Filter.prototype.toFloatString = function (number) {
    // GLSL is picky about floats, e.g. "1" is not valid
    return parseFloat(number).toExponential();
}

////////

FilterKit.Filter.Adjust = function (r, g, b, inputFilter) {
    FilterKit.Filter.call(this, inputFilter);
    r = this.toFloatString(r);
    g = this.toFloatString(g);
    b = this.toFloatString(b);
    this.adjustExpression = "vec3(" + r + ", " + g + ", " + b + ")";
}
FilterKit.subclass(FilterKit.Filter.Adjust, FilterKit.Filter);

FilterKit.Filter.library.Adjust = [
    "vec4 adjust(vec3 adjust, vec4 color) {"
    ,    "color.rgb = clamp(color.rgb * (adjust + 1.0));"
    ,    "return color;"
    ,"}"
].join("\n");
FilterKit.Filter.Adjust.prototype.libraryNames = [ "Clamp", "Adjust" ];

FilterKit.Filter.Adjust.prototype.expression = function (topExpression) {
    return "adjust(" + this.adjustExpression + ", " + topExpression + ")";
}

////////

FilterKit.Filter.Bias = function (amount, inputFilter) {
    FilterKit.Filter.call(this, inputFilter);
    this.amountExpression = this.toFloatString(amount);
}
FilterKit.subclass(FilterKit.Filter.Bias, FilterKit.Filter);

FilterKit.Filter.library.Bias = [
    "vec4 bias(float amount, vec4 color) {"
    ,    "color.rgb *= clamp(color.rgb / ((1.0 / amount - 1.9) * (0.9 - color.rgb) + 1.0));"
    ,    "return color;"
    ,"}"
].join("\n");
FilterKit.Filter.Bias.prototype.libraryNames = [ "Clamp", "Bias" ];

FilterKit.Filter.Bias.prototype.expression = function (topExpression) {
    return "bias(" + this.amountExpression + ", " + topExpression + ")";
}

////////

FilterKit.Filter.Brightness = function (factor, inputFilter) {
    FilterKit.Filter.call(this, inputFilter);
    this.factorExpression = this.toFloatString(factor);
}
FilterKit.subclass(FilterKit.Filter.Brightness, FilterKit.Filter);

FilterKit.Filter.library.Brightness = [
    "vec4 brightness(float factor, vec4 color) {"
    ,    "return vec4(clamp(color.rgb + factor), color.a);"
    ,"}"
].join("\n");
FilterKit.Filter.Brightness.prototype.libraryNames = [ "Clamp", "Brightness" ];

FilterKit.Filter.Brightness.prototype.expression = function (topExpression) {
    return "brightness(" + this.factorExpression + ", " + topExpression + ")";
}

////////

FilterKit.Filter.Contrast = function (factor, inputFilter) {
    FilterKit.Filter.call(this, inputFilter);
    this.factorExpression = this.toFloatString(factor);
}
FilterKit.subclass(FilterKit.Filter.Contrast, FilterKit.Filter);

FilterKit.Filter.library.Contrast = [
    "vec4 contrast(float factor, vec4 color) {"
    ,    "return vec4(clamp((color.rgb - 0.5) * factor + 0.5), color.a);"
    ,"}"
].join("\n");
FilterKit.Filter.Contrast.prototype.libraryNames = [ "Clamp", "Contrast" ];

FilterKit.Filter.Contrast.prototype.expression = function (topExpression) {
    return "contrast(" + this.factorExpression + ", " + topExpression + ")";
}

////////

FilterKit.Filter.DuoTone = function (darkColor, lightColor, inputFilter) {
    FilterKit.Filter.call(this, inputFilter);
    this.darkColorExpression = "vec3(" + this.toFloatString(darkColor[0]) + ", " + this.toFloatString(darkColor[1]) + ", " + this.toFloatString(darkColor[2]) + ")";
    this.lightColorExpression = "vec3(" + this.toFloatString(lightColor[0]) + ", " + this.toFloatString(lightColor[1]) + ", " + this.toFloatString(lightColor[2]) + ")";
}
FilterKit.subclass(FilterKit.Filter.DuoTone, FilterKit.Filter);

FilterKit.Filter.library.DuoTone = [
    "vec4 duotone(vec3 darkColor, vec3 lightColor, vec4 color) {"
    ,    "float luminance = dot(color.rgb, lumaCoeff.rgb);"
    ,    "color.rgb = clamp(mix(darkColor, lightColor, luminance));"
    ,    "return color;"
    ,"}"
].join("\n");
FilterKit.Filter.DuoTone.prototype.libraryNames = [ "Clamp", "Luma", "DuoTone" ];

FilterKit.Filter.DuoTone.prototype.expression = function (topExpression) {
    return "duotone(" + this.darkColorExpression + ", " + this.lightColorExpression + ", " + topExpression + ")";
}

////////

FilterKit.Filter.Fill = function (r, g, b, a, inputFilter) {
    FilterKit.Filter.call(this, inputFilter);
    r = this.toFloatString(r);
    g = this.toFloatString(g);
    b = this.toFloatString(b);
    a = this.toFloatString(a);
    this.colorExpression = "clamp(vec4(" + r + ", " + g + ", " + b + ", " + a + "))";
}
FilterKit.subclass(FilterKit.Filter.Fill, FilterKit.Filter);

FilterKit.Filter.Fill.prototype.libraryNames = [ "Clamp" ];

FilterKit.Filter.Fill.prototype.expression = function (topExpression) {
    return this.colorExpression;
}

////////

FilterKit.Filter.Gamma = function (gamma, inputFilter) {
    FilterKit.Filter.call(this, inputFilter);
    this.gammaExpression = this.toFloatString(gamma);
}
FilterKit.subclass(FilterKit.Filter.Gamma, FilterKit.Filter);

FilterKit.Filter.library.Gamma = [
    "vec4 gamma(float gamma, vec4 color) {"
    ,    "color.rgb = clamp(pow(color.rgb, vec3(gamma)));"
    ,    "return color;"
    ,"}"
].join("\n");
FilterKit.Filter.Gamma.prototype.libraryNames = [ "Clamp", "Gamma" ];

FilterKit.Filter.Gamma.prototype.expression = function (topExpression) {
    return "gamma(" + this.gammaExpression + ", " + topExpression + ")";
}

////////

FilterKit.Filter.Invert = function (inputFilter) {
    FilterKit.Filter.call(this, inputFilter);
}
FilterKit.subclass(FilterKit.Filter.Invert, FilterKit.Filter);

FilterKit.Filter.library.Invert = [
    "vec4 invert(vec4 color) {"
    ,    "return vec4(1.0 - color.rgb, color.a);"
    ,"}"
].join("\n");
FilterKit.Filter.Invert.prototype.libraryNames = [ "Invert" ];

FilterKit.Filter.Invert.prototype.expression = function (topExpression) {
    return "invert(" + topExpression + ")";
}

////////

FilterKit.Filter.Opacity = function (opacity, inputFilter) {
    FilterKit.Filter.call(this, inputFilter);
    this.opacityExpression = this.toFloatString(opacity);
}
FilterKit.subclass(FilterKit.Filter.Opacity, FilterKit.Filter);

FilterKit.Filter.library.Opacity = [
    "vec4 opacity(float opacity, vec4 color) {"
    ,    "color.a *= opacity;"
    ,    "return clamp(color);"
    ,"}"
].join("\n");
FilterKit.Filter.Opacity.prototype.libraryNames = [ "Clamp", "Opacity" ];

FilterKit.Filter.Opacity.prototype.expression = function (topExpression) {
    return "opacity(" + this.opacityExpression + ", " + topExpression + ")";
}

////////

FilterKit.Filter.Posterize = function (levels, inputFilter) {
    FilterKit.Filter.call(this, inputFilter);
    this.levelsExpression = this.toFloatString(levels);
}
FilterKit.subclass(FilterKit.Filter.Posterize, FilterKit.Filter);

FilterKit.Filter.library.Posterize = [
    "vec4 posterize(float levels, vec4 color) {"
    ,    "return vec4(clamp(floor(color.rgb * levels) / levels), color.a);"
    ,"}"
].join("\n");
FilterKit.Filter.Posterize.prototype.libraryNames = [ "Clamp", "Posterize" ];

FilterKit.Filter.Posterize.prototype.expression = function (topExpression) {
    return "posterize(" + this.levelsExpression + ", " + topExpression + ")";
}

////////

FilterKit.Filter.Rescale = function (darkColor, lightColor, inputFilter) {
    FilterKit.Filter.call(this, inputFilter);
    this.darkColorExpression = "vec3(" + this.toFloatString(darkColor[0]) + ", " + this.toFloatString(darkColor[1]) + ", " + this.toFloatString(darkColor[2]) + ")";
    this.lightColorExpression = "vec3(" + this.toFloatString(lightColor[0]) + ", " + this.toFloatString(lightColor[1]) + ", " + this.toFloatString(lightColor[2]) + ")";
}
FilterKit.subclass(FilterKit.Filter.Rescale, FilterKit.Filter);

FilterKit.Filter.library.Rescale = [
    "vec4 rescale(vec3 darkColor, vec3 lightColor, vec4 color) {"
    ,    "color.rgb = clamp(mix(darkColor, lightColor, color.rgb));"
    ,    "return color;"
    ,"}"
].join("\n");
FilterKit.Filter.Rescale.prototype.libraryNames = [ "Clamp", "Rescale" ];

FilterKit.Filter.Rescale.prototype.expression = function (topExpression) {
    return "rescale(" + this.darkColorExpression + ", " + this.lightColorExpression + ", " + topExpression + ")";
}

////////

FilterKit.Filter.Saturation = function (factor, inputFilter) {
    FilterKit.Filter.call(this, inputFilter);
    this.factorExpression = this.toFloatString(factor);
}
FilterKit.subclass(FilterKit.Filter.Saturation, FilterKit.Filter);

// From "OpenGL Shading Language 2nd Edition"
FilterKit.Filter.library.Saturation = [
    "vec4 saturation(float factor, vec4 color) {"
    ,    "vec3 intensity = vec3(dot(color.rgb, lumaCoeff.rgb));"
    ,    "return vec4(clamp(mix(intensity, color.rgb, factor)), color.a);"
    ,"}"
].join("\n");
FilterKit.Filter.Saturation.prototype.libraryNames = [ "Luma", "Clamp", "Saturation" ];

FilterKit.Filter.Saturation.prototype.expression = function (topExpression) {
    return "saturation(" + this.factorExpression + ", " + topExpression + ")";
}

////////

FilterKit.Filter.Sepia = function (inputFilter) {
    FilterKit.Filter.call(this, inputFilter);
}
FilterKit.subclass(FilterKit.Filter.Sepia, FilterKit.Filter);

FilterKit.Filter.library.Sepia = [
    "vec4 sepia(vec4 color) {"
    ,    "const mat3 sepia = mat3(0.393, 0.769, 0.189,"
    ,                            "0.349, 0.686,0.168,"
    ,                            "0.272, 0.534, 0.131);"
    ,    "return vec4(clamp(color.rgb * sepia), color.a);"
    ,"}"
].join("\n");
FilterKit.Filter.Sepia.prototype.libraryNames = [ "Clamp", "Sepia" ];

FilterKit.Filter.Sepia.prototype.expression = function (topExpression) {
    return "sepia(" + topExpression + ")";
}

////////

FilterKit.Filter.Vignette = function (inner, outer, color, inputFilter) {
    FilterKit.Filter.call(this, inputFilter);
    inner = parseFloat(inner);
    outer = parseFloat(outer);
    if (inner > outer) {
        var tmp = inner;
        inner = outer;
        outer = tmp;
    }
    this.innerExpression = this.toFloatString(inner);
    this.outerExpression = this.toFloatString(outer);
    if (color)
        this.colorExpression = "vec3(" + this.toFloatString(color[0]) + ", " + this.toFloatString(color[1]) + ", " + this.toFloatString(color[2]) + ")";
    else
        this.colorExpression = "vec3(0.0, 0.0, 0.0)";
}
FilterKit.subclass(FilterKit.Filter.Vignette, FilterKit.Filter);

FilterKit.Filter.library.Vignette = [
    "vec4 vignette(float inner, float outer, vec3 tintColor, vec4 color) {"
    ,    "float d = distance(texCoord, vec2(0.5, 0.5));"
    ,    "float amount = smoothstep(outer, inner, d);"
    ,    "return vec4(clamp(color.rgb * amount + tintColor * (1.0 - amount)), color.a);"
    ,"}"
].join("\n");
FilterKit.Filter.Vignette.prototype.libraryNames = [ "Clamp", "Vignette" ];

FilterKit.Filter.Vignette.prototype.expression = function (topExpression) {
    return "vignette(" + this.innerExpression + ", " + this.outerExpression + ", " + this.colorExpression + ", " + topExpression + ")";
}

////////

FilterKit.Filter.Blend = function (topFilter, bottomFilter) {
    FilterKit.Filter.call(this, topFilter);
    this.bottomFilter = bottomFilter;
}
FilterKit.subclass(FilterKit.Filter.Blend, FilterKit.Filter);

FilterKit.Filter.library.Blend = [
    "vec4 blend(vec4 topColor, vec4 bottomColor) {"
    ,    "return vec4(mix(bottomColor.rgb, topColor.rgb, topColor.a), bottomColor.a);"
    ,"}"
].join("\n");

////////

FilterKit.Filter.Addition = function (topFilter, bottomFilter) {
    FilterKit.Filter.Blend.call(this, topFilter, bottomFilter);
}
FilterKit.subclass(FilterKit.Filter.Addition, FilterKit.Filter.Blend);

FilterKit.Filter.library.Addition = [
    "vec4 addition(vec4 topColor, vec4 bottomColor) {"
    ,    "topColor.rgb = clamp(topColor.rgb + bottomColor.rgb);"
    ,    "return blend(topColor, bottomColor);"
    ,"}"
].join("\n");
FilterKit.Filter.Addition.prototype.libraryNames = [ "Clamp", "Blend", "Addition" ];

FilterKit.Filter.Addition.prototype.expression = function (topExpression, bottomExpression) {
    return "addition(" + topExpression + ", " + bottomExpression + ")";
}

////////

FilterKit.Filter.Difference = function (topFilter, bottomFilter) {
    FilterKit.Filter.Blend.call(this, topFilter, bottomFilter);
}
FilterKit.subclass(FilterKit.Filter.Difference, FilterKit.Filter.Blend);

FilterKit.Filter.library.Difference = [
    "vec4 difference(vec4 topColor, vec4 bottomColor) {"
    ,    "topColor.rgb = clamp(abs(topColor.rgb - bottomColor.rgb));"
    ,    "return blend(topColor, bottomColor);"
    ,"}"
].join("\n");
FilterKit.Filter.Difference.prototype.libraryNames = [ "Clamp", "Blend", "Difference" ];

FilterKit.Filter.Difference.prototype.expression = function (topExpression, bottomExpression) {
    return "difference(" + topExpression + ", " + bottomExpression + ")";
}

////////

FilterKit.Filter.Multiply = function (topFilter, bottomFilter) {
    FilterKit.Filter.Blend.call(this, topFilter, bottomFilter);
}
FilterKit.subclass(FilterKit.Filter.Multiply, FilterKit.Filter.Blend);

FilterKit.Filter.library.Multiply = [
    "vec4 multiply(vec4 topColor, vec4 bottomColor) {"
    ,    "topColor.rgb *= bottomColor.rgb;"
    ,    "return blend(topColor, bottomColor);"
    ,"}"
].join("\n");
FilterKit.Filter.Multiply.prototype.libraryNames = [ "Blend", "Multiply" ];

FilterKit.Filter.Multiply.prototype.expression = function (topExpression, bottomExpression) {
    return "multiply(" + topExpression + ", " + bottomExpression + ")";
}

////////

FilterKit.Filter.Normal = function (topFilter, bottomFilter) {
    FilterKit.Filter.Blend.call(this, topFilter, bottomFilter);
}
FilterKit.subclass(FilterKit.Filter.Normal, FilterKit.Filter.Blend);

FilterKit.Filter.library.Normal = [
    "vec4 normal(vec4 topColor, vec4 bottomColor) {"
    ,    "return blend(topColor, bottomColor);"
    ,"}"
].join("\n");
FilterKit.Filter.Normal.prototype.libraryNames = [ "Blend", "Normal" ];

FilterKit.Filter.Normal.prototype.expression = function (topExpression, bottomExpression) {
    return "normal(" + topExpression + ", " + bottomExpression + ")";
}

////////

FilterKit.Filter.Overlay = function (topFilter, bottomFilter) {
    FilterKit.Filter.Blend.call(this, topFilter, bottomFilter);
}
FilterKit.subclass(FilterKit.Filter.Overlay, FilterKit.Filter.Blend);

// From "OpenGL Shading Language 2nd Edition"
FilterKit.Filter.library.Overlay = [
    "vec4 overlay(vec4 topColor, vec4 bottomColor) {"
    ,    "float luminance = dot(bottomColor.rgb, lumaCoeff.rgb);"
    ,    "if (luminance < 0.45) {"
    ,        "topColor.rgb *= 2.0 * bottomColor.rgb;"
    ,        "return blend(topColor, bottomColor);"
    ,    "}"
    ,    "else if (luminance > 0.55) {"
    ,        "topColor.rgb = 1.0 - 2.0 * (1.0 - topColor.rgb) * (1.0 - bottomColor.rgb);"
    ,        "return blend(topColor, bottomColor);"
    ,    "}"
    ,    "else {"
    ,        "vec3 result1 = 2.0 * topColor.rgb * bottomColor.rgb;"
    ,        "vec3 result2 = 1.0 - 2.0 * (1.0 - topColor.rgb)  * (1.0 - bottomColor.rgb);"
    ,        "topColor.rgb = mix(result1, result2, (luminance - 0.45) * 10.0);"
    ,        "return blend(topColor, bottomColor);"
    ,    "}"
    ,"}"
].join("\n");
FilterKit.Filter.Overlay.prototype.libraryNames = [ "Blend", "Luma", "Overlay" ];

FilterKit.Filter.Overlay.prototype.expression = function (topExpression, bottomExpression) {
    return "overlay(" + topExpression + ", " + bottomExpression + ")";
}

////////

FilterKit.Filter.Screen = function (topFilter, bottomFilter) {
    FilterKit.Filter.Blend.call(this, topFilter, bottomFilter);
}
FilterKit.subclass(FilterKit.Filter.Screen, FilterKit.Filter.Blend);

FilterKit.Filter.library.Screen = [
    "vec4 screen(vec4 topColor, vec4 bottomColor) {"
    ,    "topColor.rgb = 1.0 - ((1.0 - topColor.rgb) * (1.0 - bottomColor.rgb));"
    ,    "return blend(topColor, bottomColor);"
    ,"}"
].join("\n");
FilterKit.Filter.Screen.prototype.libraryNames = [ "Blend", "Screen" ];

FilterKit.Filter.Screen.prototype.expression = function (topExpression, bottomExpression) {
    return "screen(" + topExpression + ", " + bottomExpression + ")";
}

////////

FilterKit.Filter.SoftLight = function (topFilter, bottomFilter) {
    FilterKit.Filter.Blend.call(this, topFilter, bottomFilter);
}
FilterKit.subclass(FilterKit.Filter.SoftLight, FilterKit.Filter.Blend);

FilterKit.Filter.library.SoftLight = [
    "vec4 softLight(vec4 topColor, vec4 bottomColor) {"
    ,    "topColor.rgb = clamp(2.0 * bottomColor.rgb * topColor.rgb + bottomColor.rgb * bottomColor.rgb - 2.0 * bottomColor.rgb * bottomColor.rgb * topColor.rgb);"
    ,    "return blend(topColor, bottomColor);"
    ,"}"
].join("\n");
FilterKit.Filter.SoftLight.prototype.libraryNames = [ "Clamp", "Blend", "SoftLight" ];

FilterKit.Filter.SoftLight.prototype.expression = function (topExpression, bottomExpression) {
    return "softLight(" + topExpression + ", " + bottomExpression + ")";
}

////////

FilterKit.Filter.Convolve = function (kernelName, topFilter) {
    FilterKit.Filter.call(this, topFilter);
    this.kernelName = kernelName;
}
FilterKit.subclass(FilterKit.Filter.Convolve, FilterKit.Filter);

// dimension - convolution kernel dimension (e.g. 3 for 3x3). Must be odd.
FilterKit.Filter.Convolve.convolveSource = function (dimension) {
    var suffix = dimension + "x" + dimension;
    return [
        "const int ConvolveKernelDimension" + suffix + " = " + dimension + ";"
        ,"vec4 convolve(float kernel[ConvolveKernelDimension" + suffix + " * ConvolveKernelDimension" + suffix + "], sampler2D sampler) {"
        ,    "vec2 step = fwidth(texCoord);"
        ,    "int offset = (ConvolveKernelDimension" + suffix + " - 1) / 2;"
        ,    "vec4 total = vec4(0.0);"
        ,    "vec2 uvOffset;"
        ,    "for (int y = 0; y < ConvolveKernelDimension" + suffix + "; y++) {"
        ,        "uvOffset.y = step.y * float(y - offset);"
        ,        "for (int x = 0; x < ConvolveKernelDimension" + suffix + "; x++) {"
        ,            "uvOffset.x = step.x * float(x - offset);"
        ,            "total += texture2D(sampler, texCoord + uvOffset) * kernel[y * ConvolveKernelDimension" + suffix + " + x];"
        ,        "}"
        ,    "}"
        ,    "return clamp(total);"
        ,"}"
    ].join("\n");
}

//XXX pass in canvas width/height explicitly to all shaders instead of using fwidth()?
FilterKit.Filter.Convolve.prototype.extensionNames = { "GL_OES_standard_derivatives": "OES_standard_derivatives" };


FilterKit.Filter.Convolve.prototype.expression = function (topExpression) {
    return "convolve(" + this.kernelName + ", " + topExpression + ")";
}

////////

FilterKit.Filter.Convolve3x3 = function (kernelName, topFilter) {
    FilterKit.Filter.Convolve.call(this, kernelName, topFilter);
}
FilterKit.subclass(FilterKit.Filter.Convolve3x3, FilterKit.Filter.Convolve);

FilterKit.Filter.library.Convolve3x3 = FilterKit.Filter.Convolve.convolveSource(3);
FilterKit.Filter.Convolve3x3.prototype.libraryNames = [ "Clamp", "Convolve3x3" ];

////////

FilterKit.Filter.Blur = function (topFilter) {
    FilterKit.Filter.Convolve3x3.call(this, "blurKernel", topFilter);
}
FilterKit.subclass(FilterKit.Filter.Blur, FilterKit.Filter.Convolve3x3);

FilterKit.Filter.library.Blur =
    "uniform float blurKernel[ConvolveKernelDimension3x3 * ConvolveKernelDimension3x3];"
FilterKit.Filter.Blur.prototype.libraryNames = FilterKit.Filter.Convolve3x3.prototype.libraryNames.concat("Blur");
FilterKit.Filter.Blur.prototype.uniformValues = {
    "blurKernel[0]": [1/14, 2/14, 1/14,
                      2/14, 2/14, 2/14,
                      1/14, 2/14, 1/14]
};

////////

FilterKit.Filter.BoxBlur = function (topFilter) {
    FilterKit.Filter.Convolve3x3.call(this, "boxBlurKernel", topFilter);
}
FilterKit.subclass(FilterKit.Filter.BoxBlur, FilterKit.Filter.Convolve3x3);

FilterKit.Filter.library.BoxBlur =
    "uniform float boxBlurKernel[ConvolveKernelDimension3x3 * ConvolveKernelDimension3x3];"
FilterKit.Filter.BoxBlur.prototype.libraryNames = FilterKit.Filter.Convolve3x3.prototype.libraryNames.concat("BoxBlur");
FilterKit.Filter.BoxBlur.prototype.uniformValues = {
    "boxBlurKernel[0]": [1/9, 1/9, 1/9,
                         1/9, 1/9, 1/9,
                         1/9, 1/9, 1/9]
};

////////

FilterKit.Filter.Bump = function (topFilter) {
    FilterKit.Filter.Convolve3x3.call(this, "bumpKernel", topFilter);
}
FilterKit.subclass(FilterKit.Filter.Bump, FilterKit.Filter.Convolve3x3);

FilterKit.Filter.library.Bump =
    "uniform float bumpKernel[ConvolveKernelDimension3x3 * ConvolveKernelDimension3x3];"
FilterKit.Filter.Bump.prototype.libraryNames = FilterKit.Filter.Convolve3x3.prototype.libraryNames.concat("Bump");
FilterKit.Filter.Bump.prototype.uniformValues = {
    "bumpKernel[0]": [-1, -1, 0,
                      -1, 1, 1,
                      0, 1, 1]
};

////////

FilterKit.Filter.Emboss = function (topFilter) {
    FilterKit.Filter.Convolve3x3.call(this, "embossKernel", topFilter);
}
FilterKit.subclass(FilterKit.Filter.Emboss, FilterKit.Filter.Convolve3x3);

FilterKit.Filter.library.Emboss =
    "uniform float embossKernel[ConvolveKernelDimension3x3 * ConvolveKernelDimension3x3];"
FilterKit.Filter.Emboss.prototype.libraryNames = FilterKit.Filter.Convolve3x3.prototype.libraryNames.concat("Emboss");
FilterKit.Filter.Emboss.prototype.uniformValues = {
    "embossKernel[0]": [-2, -1, 0,
                        -1, 1, 1,
                        0, 1, 2]
};

////////

FilterKit.Filter.Sharpen = function (topFilter) {
    FilterKit.Filter.Convolve3x3.call(this, "sharpenKernel", topFilter);
}
FilterKit.subclass(FilterKit.Filter.Sharpen, FilterKit.Filter.Convolve3x3);

FilterKit.Filter.library.Sharpen =
    "uniform float sharpenKernel[ConvolveKernelDimension3x3 * ConvolveKernelDimension3x3];"
FilterKit.Filter.Sharpen.prototype.libraryNames = FilterKit.Filter.Convolve3x3.prototype.libraryNames.concat("Sharpen");
FilterKit.Filter.Sharpen.prototype.uniformValues = {
    "sharpenKernel[0]": [0, -0.2, 0,
                         -0.2, 1.8, -0.2,
                         0, -0.2, 0]
};

////////

FilterKit.Filter.SobelEdge = function (topFilter) {
    FilterKit.Filter.Convolve3x3.call(this, null, topFilter);
}
FilterKit.subclass(FilterKit.Filter.SobelEdge, FilterKit.Filter.Convolve3x3);

FilterKit.Filter.library.SobelEdge = [
    "uniform float sobelEdgeKernelH[ConvolveKernelDimension3x3 * ConvolveKernelDimension3x3];"
    ,"uniform float sobelEdgeKernelV[ConvolveKernelDimension3x3 * ConvolveKernelDimension3x3];"
    ,"vec4 sobelEdge(sampler2D sampler) {"
    ,    "vec3 colorH = convolve(sobelEdgeKernelH, sampler).rgb;"
    ,    "vec3 colorV = convolve(sobelEdgeKernelV, sampler).rgb;"
    //XXX preserve alpha of sampler?
    ,    "return clamp(vec4(sqrt(colorH * colorH + colorV * colorV), 1.0));"
    ,"}"
].join("\n");
FilterKit.Filter.SobelEdge.prototype.libraryNames = FilterKit.Filter.Convolve3x3.prototype.libraryNames.concat("SobelEdge");
FilterKit.Filter.SobelEdge.prototype.uniformValues = {
    "sobelEdgeKernelH[0]": [-1, -2, -1,
                            0, 0, 0,
                            1, 2, 1],
    "sobelEdgeKernelV[0]": [-1, 0, 1
                            -2, 0, 2,
                            -1, 0, 1],
};

FilterKit.Filter.SobelEdge.prototype.expression = function (topExpression) {
    return "sobelEdge(" + topExpression + ")";
}

////////

FilterKit.Filter.SuperSharpen = function (topFilter) {
    FilterKit.Filter.Convolve3x3.call(this, "superSharpenKernel", topFilter);
}
FilterKit.subclass(FilterKit.Filter.SuperSharpen, FilterKit.Filter.Convolve3x3);

FilterKit.Filter.library.SuperSharpen =
    "uniform float superSharpenKernel[ConvolveKernelDimension3x3 * ConvolveKernelDimension3x3];"
FilterKit.Filter.SuperSharpen.prototype.libraryNames = FilterKit.Filter.Convolve3x3.prototype.libraryNames.concat("SuperSharpen");
FilterKit.Filter.SuperSharpen.prototype.uniformValues = {
    "superSharpenKernel[0]": [0, -1, 0,
                              -1, 5, -1,
                              0, -1, 0]
};

////////

FilterKit.Filter.Convolve5x5 = function (kernelName, topFilter) {
    FilterKit.Filter.Convolve.call(this, kernelName, topFilter);
}
FilterKit.subclass(FilterKit.Filter.Convolve5x5, FilterKit.Filter.Convolve);

FilterKit.Filter.library.Convolve5x5 = FilterKit.Filter.Convolve.convolveSource(5);
FilterKit.Filter.Convolve5x5.prototype.libraryNames = [ "Clamp", "Convolve5x5" ];

////////

FilterKit.Filter.GaussianBlur = function (topFilter) {
    FilterKit.Filter.Convolve5x5.call(this, "gaussianBlurKernel", topFilter);
}
FilterKit.subclass(FilterKit.Filter.GaussianBlur, FilterKit.Filter.Convolve5x5);

FilterKit.Filter.library.GaussianBlur =
    "uniform float gaussianBlurKernel[ConvolveKernelDimension5x5 * ConvolveKernelDimension5x5];"
FilterKit.Filter.GaussianBlur.prototype.libraryNames = FilterKit.Filter.Convolve5x5.prototype.libraryNames.concat("GaussianBlur");
FilterKit.Filter.GaussianBlur.prototype.uniformValues = {
    "gaussianBlurKernel[0]": [1/273, 4/273, 7/273, 4/273, 1/273,
                              4/273, 16/273, 26/273, 16/273, 4/273,
                              7/273, 26/273, 41/273, 26/273, 7/273,
                              4/273, 16/273, 26/273, 16/273, 4/273,
                              1/273, 4/273, 7/273, 4/273, 1/273]
};
