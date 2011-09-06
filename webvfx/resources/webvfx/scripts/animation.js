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
WebVfx.Animation = function (animationData) {
    if (!animationData)
        return;
    this.range = animationData['range'];

    this.locationXAnimation = this.processAnimation(animationData['locationX']);
    this.locationYAnimation = this.processAnimation(animationData['locationY']);
    this.locationZAnimation = this.processAnimation(animationData['locationZ']);
    this.rotationXAnimation = this.processAnimation(animationData['rotationX']);
    this.rotationYAnimation = this.processAnimation(animationData['rotationY']);
    this.rotationZAnimation = this.processAnimation(animationData['rotationZ']);

    // Horizontal field of view in radians
    this.horizontalFOV = animationData['horizontalFOV'];

    this.evaluateTime(0);
}

WebVfx.Animation.prototype.processAnimation = function(animation) {
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

// Evaluate for x which must be in the animations range
// After evaluating, location[XYZ] and rotation[XYZ] will be updated.
// Rotation are Blender Euler angles of order XYZ - but Blender XYZ
// is really ZYX order.
WebVfx.Animation.prototype.evaluate = function(x) {
    if (this.currentX == x)
        return;
    this.currentX = x;

    this.rotationX = this.rotationXAnimation.evaluate(x);
    this.rotationY = this.rotationYAnimation.evaluate(x);
    this.rotationZ = this.rotationZAnimation.evaluate(x);

    this.locationX = this.locationXAnimation.evaluate(x);
    this.locationY = this.locationYAnimation.evaluate(x);
    this.locationZ = this.locationZAnimation.evaluate(x);
}

// time is normalized 0..1
WebVfx.Animation.prototype.evaluateTime = function(time) {
    // Find x corresponding to time
    this.evaluate(this.range[0] + time * (this.range[1] - this.range[0] + 1));
}

// Compute vertical field of view in radians,
// given viewport aspect (width/height)
WebVfx.Animation.prototype.verticalFOV = function(aspect) {
    return 2 * Math.atan(Math.tan(this.horizontalFOV / 2) / aspect);
}

WebVfx.Animation.prototype.radians2degrees = function (radians) {
    return radians * 180 / Math.PI;
}
