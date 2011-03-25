// QML library
.pragma library

// range is an array of [beginX,endX] x coordinates that the endpoints
// of this segment cover.
// points is an array of four Bezier [x,y] control points,
// [[ax,ay],[bx,by],[cx,cy],[dx,dy]]
// See http://www.flong.com/texts/code/shapers_bez/
function BezierSegment(range, points) {
    this.range = range;
    this.xCoefficients = this.polynomialCoefficients(points, 0);
    this.yCoefficients = this.polynomialCoefficients(points, 1);
}

BezierSegment.prototype.TOLERANCE = 0.000001;
BezierSegment.prototype.ITERATIONS = 5;

// Return y value for given x.
BezierSegment.prototype.evaluate = function(x) {
    // Solve for t given x (using Newton-Raphson),
    // then solve for y given t.
    // For first guess, linearly interpolate to get t.
    var t = (this.range[1] - this.range[0]) / x;
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
BezierSegment.prototype.polynomialCoefficients = function(p, i) {
    return [p[3][i] - 3 * p[2][i] + 3 * p[1][i] - p[0][i],
            3 * p[2][i] - 6 * p[1][i] + 3 * p[0][i],
            3 * p[1][i] - 3 * p[0][i],
            p[0][i]];
}

// Evaluate cubic polynomial at time t.
// c are the polynomial coefficients [a,b,c,d]
BezierSegment.prototype.evaluatePolynomial = function(t, c) {
    // Use Horners rule for polynomial evaluation
    return ((c[0] * t + c[1]) * t + c[2]) * t + c[3];
}

// Return slope given t for coefficients c
BezierSegment.prototype.slope = function(t, c) {
    return 1.0 / (3.0 * c[0] * t * t + 2.0 * c[1] * t + c[2]);
}

BezierSegment.prototype.clamp = function(v) {
    if (v < 0.0)
        return 0.0;
    else if (v > 1.0)
        return 1.0;
    return v;
}


// segments is an ordered array of non overlapping BezierSegments
function BezierCurve(segments) {
    this.segments = segments;
    this.currentSegment = null;
}

// Binary search to find segment that contains x
BezierCurve.prototype.findSegment = function(x) {
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

BezierCurve.prototype.evaluate = function(x) {
    // Find current segment if we are out of range
    if (this.currentSegment == null || x < this.currentSegment.range[0] ||
        x > this.currentSegment.range[1])
        this.currentSegment = this.findSegment(x);

    return this.currentSegment.evaluate(x);
}


function ConstantValue(value) {
    this.value = value;
}

ConstantValue.prototype.evaluate = function(x) {
    return this.value;
}


// animation is the animation data exported from Blender webvfx-camera.py tool
function CameraAnimation(animation) {
    this.range = animation['range'];

    this.locationX = this.processAnimation(animation['locationX']);
    this.locationY = this.processAnimation(animation['locationY']);
    this.locationZ = this.processAnimation(animation['locationZ']);
    this.rotationX = this.processAnimation(animation['rotationX']);
    this.rotationY = this.processAnimation(animation['rotationY']);
    this.rotationZ = this.processAnimation(animation['rotationZ']);

    this.upVector = [0,1,0];
    this.lookAt = [0,0,-1];
    this.eye = [0,0,0];
}

CameraAnimation.prototype.processAnimation = function(animation) {
    if (typeof(animation) == 'number')
        return new ConstantValue(animation);
    else {
        var segments = [];
        for (var i = 0; i < animation.length; i++) {
            segments[i] = new BezierSegment(animation[i]['range'],
                                            animation[i]['bezierPoints']);
        }
        return new BezierCurve(segments);
    }
    return null;
}

// t is 0..1
// After evaluating, upVector, lookAt and eye will be updated.
CameraAnimation.prototype.evaluate = function(t) {
    // Find x corresponding to t
    var x = this.range[0] + t * (this.range[1] - this.range[0] + 1);

    this.eye[0] = this.locationX.evaluate(x);
    this.eye[1] = this.locationY.evaluate(x);
    this.eye[2] = this.locationZ.evaluate(x);

    var rotX = this.rotationX.evaluate(x);
    var rotY = this.rotationY.evaluate(x);
    var rotZ = this.rotationZ.evaluate(x);

    // Blender Euler order is XYZ, so angles are applied in ZYX order.
    // Using the ZYX matrix from, but transposed (so passive transformation
    // instead of active)
    // http://en.wikipedia.org/wiki/Euler_angles#Matrix_orientation
    var cx = Math.cos(rotX);
    var cy = Math.cos(rotY);
    var cz = Math.cos(rotZ);
    var sx = Math.sin(rotX);
    var sy = Math.sin(rotY);
    var sz = Math.sin(rotZ);

    var cc = cx*cz;
    var cs = cx*sz;
    var sc = sx*cz;
    var ss = sx*sz;

    var m10 = sy*sc-cs;
    var m11 = sy*ss+cc;
    var m12 = cy*sx;
    var m20 = sy*cc+ss;
    var m21 = sy*cs-sc;
    var m22 = cy*cx;

    // Up vector can just be read out of the matrix (y axis)
    // (m10, m11, m12)
    this.lookAt[0] = this.eye[0] - m20;
    this.lookAt[1] = this.eye[1] - m21;
    this.lookAt[2] = this.eye[2] - m22;
    // Up vector can just be read out of the matrix (y axis)
    // (m10, m11, m12)
    this.upVector[0] = m10;
    this.upVector[1] = m11;
    this.upVector[2] = m12;
}

// QML doesn't allow global variables, so provide one here to store the instance
var camera = null;

function initCameraAnimation(animation) {
    camera = new CameraAnimation(animation);
}

// Evaluate global CameraAnimation at time t and update the QML Camera instance
function updateQmlCamera(qmlCamera, t) {
    camera.evaluate(t);
    qmlCamera.eye = Qt.vector3d(camera.eye[0], camera.eye[1], camera.eye[2]);
    qmlCamera.center = Qt.vector3d(camera.lookAt[0], camera.lookAt[1], camera.lookAt[2]);
    qmlCamera.upVector = Qt.vector3d(camera.upVector[0], camera.upVector[1], camera.upVector[2]);
}