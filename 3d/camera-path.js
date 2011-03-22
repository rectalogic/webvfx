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
    var t;
    if (x == this.range[0])
        t = 0;
    else if (x == this.range[1])
        t = 1;
    else {
        // Solve for t given x (using Newton-Raphson),
        // then solve for y given t.
        // For first guess, linearly interpolate to get t.
        t = (this.range[1] - this.range[0]) / x;
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
    }
    return this.evaluatePolynomial(t, this.yCoefficients);
}

// Return polynomial coefficients [a,b,c,d] for control
// points in p (array of 4 [x,y] control points),
// for coordinate i.
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


// range is the [beginX,endX] range of x coordinates this curve covers.
// segments is an ordered array of non overlapping BezierSegments
function BezierCurve(range, segments) {
    this.range = range;
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
    return null;
}

BezierCurve.prototype.evaluate = function(x) {
    // Find current segment if we are out of range
    if (this.currentSegment == null || x < this.currentSegment.range[0] ||
        x > this.currentSegment.range[1])
        this.currentSegment = this.findSegment(x);

    return this.currentSegment.evaluate(x);
}