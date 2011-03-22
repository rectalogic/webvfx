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

// Return y value for given x.
BezierSegment.prototype.evaluate = function(x) {
    //XXX optimize when at t=0 or 1 (i.e. end of range)
    //XXX divide by zero if x is zero...
    // Solve for t given x (using Newton-Raphelson), then solve for y given t.
    // For first guess, linearly interpolate to get t.
    var t = (this.range[1] - this.range[0]) / x;
    for (var i = 0; i < 5; i++) {
        var currentX = this.evaluatePolynomial(t, this.xCoefficients);
        var currentSlope = this.slope(t, this.xCoefficients);
        t -= (currentX - x) * currentSlope;
        t = this.clamp(t);
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

BezierCurve.prototype.evaluate = function(x) {
    // Check if we're still inside the current segment
    if (this.currentSegment && x >= this.currentSegment.range[0] &&
        x <= this.currentSegment.range[1])
        return this.currentSegment.evaluate(x);

//XXX binary search and set currentSegment
}