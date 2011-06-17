// Copyright (c) 2011 Hewlett-Packard Development Company, L.P. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

var WebVfx = WebVfx || {};

// range is an array of [beginX,endX] x coordinates that the endpoints
// of this segment cover.
// points is an array of four Bezier [x,y] control points,
// [[ax,ay],[bx,by],[cx,cy],[dx,dy]]
// See http://www.flong.com/texts/code/shapers_bez/
WebVfx.BezierSegment = function (range, points) {
    this.range = range;
    this.xCoefficients = this.polynomialCoefficients(points, 0);
    this.yCoefficients = this.polynomialCoefficients(points, 1);
}

WebVfx.BezierSegment.prototype.TOLERANCE = 0.000001;
WebVfx.BezierSegment.prototype.ITERATIONS = 5;

// Return y value for given x.
WebVfx.BezierSegment.prototype.evaluate = function(x) {
    // Solve for t given x (using Newton-Raphson),
    // then solve for y given t.
    // For first guess, linearly interpolate to get t.
    var t = x / (this.range[1] - this.range[0] + 1);
    var oldt = t;
    for (var i = 0; i < this.ITERATIONS; i++) {
        var currentX = this.evaluatePolynomial(t, this.xCoefficients);
        var currentSlope = this.slope(t, this.xCoefficients);
        t -= (currentX - x) * currentSlope;
        t = this.clamp(t);
        if (Math.abs(oldt - t) <= this.TOLERANCE)
            break;
        oldt = t;
    }
    return this.evaluatePolynomial(t, this.yCoefficients);
}

// Return polynomial coefficients [a,b,c,d] for control
// points in p (array of 4 [x,y] control points),
// for coordinate i (0 for x, 1 for y).
// See http://www.cs.binghamton.edu/~reckert/460/bezier.htm
WebVfx.BezierSegment.prototype.polynomialCoefficients = function(p, i) {
    return [p[3][i] - 3 * p[2][i] + 3 * p[1][i] - p[0][i],
            3 * p[2][i] - 6 * p[1][i] + 3 * p[0][i],
            3 * p[1][i] - 3 * p[0][i],
            p[0][i]];
}

// Evaluate cubic polynomial at time t.
// c are the polynomial coefficients [a,b,c,d]
WebVfx.BezierSegment.prototype.evaluatePolynomial = function(t, c) {
    // Use Horners rule for polynomial evaluation
    return ((c[0] * t + c[1]) * t + c[2]) * t + c[3];
}

// Return slope given t for coefficients c
WebVfx.BezierSegment.prototype.slope = function(t, c) {
    return 1.0 / (3.0 * c[0] * t * t + 2.0 * c[1] * t + c[2]);
}

WebVfx.BezierSegment.prototype.clamp = function(v) {
    if (v < 0.0)
        return 0.0;
    else if (v > 1.0)
        return 1.0;
    return v;
}

///////////

// segments is an ordered array of non overlapping BezierSegments
WebVfx.BezierCurve = function (segments) {
    this.segments = segments;
    this.currentSegment = null;
}

// Binary search to find segment that contains x
WebVfx.BezierCurve.prototype.findSegment = function(x) {
    var startIndex = 0;
    var stopIndex = this.segments.length - 1;
    var middleIndex = Math.floor((stopIndex + startIndex) / 2);
    while (startIndex < stopIndex) {
        var segment = this.segments[middleIndex];
        if (x < segment.range[0])
            stopIndex = middleIndex - 1;
        else if (x > segment.range[1])
            startIndex = middleIndex + 1;
        else
            return segment;
        middleIndex = Math.floor((stopIndex + startIndex) / 2);
    }
    // We failed to find the segment, return first or last.
    // Segment will clamp x to it's range.
    return this.segments[middleIndex];
}

WebVfx.BezierCurve.prototype.evaluate = function(x) {
    // Find current segment if we are out of range
    if (this.currentSegment == null || x < this.currentSegment.range[0] ||
        x > this.currentSegment.range[1])
        this.currentSegment = this.findSegment(x);

    return this.currentSegment.evaluate(x);
}

///////////

WebVfx.ConstantValue = function (value) {
    this.value = value;
}

WebVfx.ConstantValue.prototype.evaluate = function(x) {
    return this.value;
}

///////////

// animation is the animation data exported from Blender webvfx-camera.py add-on
WebVfx.CameraAnimation = function (animation) {
    if (!animation)
        return;
    this.range = animation['range'];

    this.locationXAnimation = this.processAnimation(animation['locationX']);
    this.locationYAnimation = this.processAnimation(animation['locationY']);
    this.locationZAnimation = this.processAnimation(animation['locationZ']);
    this.rotationXAnimation = this.processAnimation(animation['rotationX']);
    this.rotationYAnimation = this.processAnimation(animation['rotationY']);
    this.rotationZAnimation = this.processAnimation(animation['rotationZ']);

    this.upVector = new Array(3);
    this.lookAt = new Array(3);
    this.eye = new Array(3);

    // Rest of matrix elements will be computed in evaluate()
    this.m41 = 0;
    this.m42 = 0;
    this.m43 = 0;
    this.m44 = 1;

    // Horizontal field of view in radians
    this.horizontalFOV = animation['horizontalFOV'];

    this.evaluate(0);
}

WebVfx.CameraAnimation.prototype.processAnimation = function(animation) {
    if (typeof(animation) == 'number')
        return new WebVfx.ConstantValue(animation);
    else {
        var segments = [];
        for (var i = 0; i < animation.length; i++) {
            segments[i] = new WebVfx.BezierSegment(animation[i]['range'],
                                                   animation[i]['bezierPoints']);
        }
        return new WebVfx.BezierCurve(segments);
    }
    return null;
}

// t is 0..1
// After evaluating, upVector, lookAt and eye and matrix elements
// will be updated.
WebVfx.CameraAnimation.prototype.evaluate = function(t) {
    if (this.time == t)
        return;
    this.time = t;

    // Find x corresponding to t
    var x = this.range[0] + t * (this.range[1] - this.range[0] + 1);

    this.rotationX = this.rotationXAnimation.evaluate(x);
    this.rotationY = this.rotationYAnimation.evaluate(x);
    this.rotationZ = this.rotationZAnimation.evaluate(x);

    this.locationX = this.locationXAnimation.evaluate(x);
    this.locationY = this.locationYAnimation.evaluate(x);
    this.locationZ = this.locationZAnimation.evaluate(x);

    // Blender Euler order is ZYX order.
    // Using the ZYX matrix from, but transposed (so passive transformation
    // instead of active)
    // http://en.wikipedia.org/wiki/Euler_angles#Matrix_orientation
    var cx = Math.cos(this.rotationX);
    var cy = Math.cos(this.rotationY);
    var cz = Math.cos(this.rotationZ);
    var sx = Math.sin(this.rotationX);
    var sy = Math.sin(this.rotationY);
    var sz = Math.sin(this.rotationZ);

    var cc = cx*cz;
    var cs = cx*sz;
    var sc = sx*cz;
    var ss = sx*sz;

    this.m11 = cy*cz;
    this.m21 = cy*sz;
    this.m31 = -sy;

    this.m12 = sy*sc-cs;
    this.m22 = sy*ss+cc;
    this.m32 = cy*sx;

    this.m13 = sy*cc+ss;
    this.m23 = sy*cs-sc;
    this.m33 = cy*cx;

    this.m14 = this.locationX;
    this.m24 = this.locationY;
    this.m34 = this.locationZ;

    // Eye is at translation position
    this.eye[0] = this.m14;
    this.eye[1] = this.m24;
    this.eye[2] = this.m34;

    // Direction is eye looking down Z (m13, m23, m33).
    // LookAt is (eye - direction)
    this.lookAt[0] = this.eye[0] - this.m13;
    this.lookAt[1] = this.eye[1] - this.m23;
    this.lookAt[2] = this.eye[2] - this.m33;

    // Up vector can just be read out of the matrix (y axis)
    // (m12, m22, m32)
    this.upVector[0] = this.m12;
    this.upVector[1] = this.m22;
    this.upVector[2] = this.m32;
}

// Compute vertical field of view in radians,
// given viewport aspect (width/height)
WebVfx.CameraAnimation.prototype.verticalFOV = function(aspect) {
    return 2 * Math.atan(Math.tan(this.horizontalFOV / 2) / aspect);
}

WebVfx.CameraAnimation.prototype.radians2degrees = function (radians) {
    return radians * 180 / Math.PI;
}

///////////

// If using https://github.com/mrdoob/three.js then create camera subclass
if (THREE && THREE.Camera) {
    WebVfx.AnimatedCamera = function (aspect, nearPlane, farPlane, animationData) {
        var ca = new WebVfx.CameraAnimation(animationData);
        this.cameraAnimation = ca;
        var fov = ca.radians2degrees(ca.verticalFOV(aspect));
        THREE.Camera.call(this, fov, aspect, nearPlane, farPlane);
        this.useTarget = false;
    };

    WebVfx.AnimatedCamera.prototype = new THREE.Camera();
    WebVfx.AnimatedCamera.prototype.constructor = WebVfx.AnimatedCamera;

    WebVfx.AnimatedCamera.prototype.setAnimationTime = function (time) {
        this.cameraAnimation.evaluate(time);
    };

    // Override
    WebVfx.AnimatedCamera.prototype.updateMatrix = function () {
        var ca = this.cameraAnimation;
        this.matrix.set(
            ca.m11, ca.m12, ca.m13, ca.m14,
            ca.m21, ca.m22, ca.m23, ca.m24,
            ca.m31, ca.m32, ca.m33, ca.m34,
            ca.m41, ca.m42, ca.m43, ca.m44
        );
        this.matrixWorldNeedsUpdate = true;
    };
}