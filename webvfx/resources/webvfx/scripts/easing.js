// Copyright (c) 2011 Hewlett-Packard Development Company, L.P. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
// Based on easing equations:
//  Copyright (c) 2001 Robert Penner. All rights reserved.
//  http://www.robertpenner.com/easing/

var WebVfx = WebVfx || {};

WebVfx.Easing = {};

WebVfx.Easing.Linear = function (begin, change, duration) {
    this.begin = begin;
    this.change = change;
    this.duration = duration;
}

WebVfx.Easing.Linear.prototype.ease = function (time) {
    return this.change * time / this.duration + this.begin;
}

WebVfx.Easing.Quadratic = function (begin, change, duration) {
    this.begin = begin;
    this.change = change;
    this.duration = duration;
}

WebVfx.Easing.Quadratic.prototype.easeIn = function (time) {
    return this.change * (time /= this.duration) * time + this.begin;
}

WebVfx.Easing.Quadratic.prototype.easeOut = function (time) {
    return -this.change * (time /= this.duration) * (time - 2) + this.begin;
}

WebVfx.Easing.Quadratic.prototype.easeInOut = function (time) {
    if ((time /= this.duration / 2) < 1)
        return this.change / 2 * time * time + this.begin;
    return -this.change / 2 * ((--time) * (time - 2) - 1) + this.begin;
}

WebVfx.Easing.Cubic = function (begin, change, duration) {
    this.begin = begin;
    this.change = change;
    this.duration = duration;
}

WebVfx.Easing.Cubic.prototype.easeIn = function (time) {
    return this.change * (time /= this.duration) * time * time + this.begin;
}

WebVfx.Easing.Cubic.prototype.easeOut = function (time) {
    return this.change * ((time = time / this.duration - 1) * time * time + 1) + this.begin;
}

WebVfx.Easing.Cubic.prototype.easeInOut = function (time) {
    if ((time /= this.duration / 2) < 1)
        return this.change / 2 * time * time * time + this.begin;
    return this.change / 2 * ((time -= 2) * time * time + 2) + this.begin;
}

WebVfx.Easing.Quartic = function (begin, change, duration) {
    this.begin = begin;
    this.change = change;
    this.duration = duration;
}

WebVfx.Easing.Quartic.prototype.easeIn = function (time) {
    return this.change * (time /= this.duration) * time * time * time + this.begin;
}

WebVfx.Easing.Quartic.prototype.easeOut = function (time) {
    return -this.change * ((time = time / this.duration - 1) * time * time * time - 1) + this.begin;
}

WebVfx.Easing.Quartic.prototype.easeInOut = function (time) {
    if ((time /= this.duration / 2) < 1)
        return this.change / 2 * time * time * time * time + this.begin;
    return -this.change / 2 * ((time -= 2) * time * time * time - 2) + this.begin;
}

WebVfx.Easing.Quintic = function (begin, change, duration) {
    this.begin = begin;
    this.change = change;
    this.duration = duration;
}

WebVfx.Easing.Quintic.prototype.easeIn = function (time) {
    return this.change * (time /= this.duration) * time * time * time * time + this.begin;
}

WebVfx.Easing.Quintic.prototype.easeOut = function (time) {
    return this.change * ((time = time / this.duration - 1) * time * time * time * time + 1) + this.begin;
}

WebVfx.Easing.Quintic.prototype.easeInOut = function (time) {
    if ((time /= this.duration / 2) < 1)
        return this.change / 2 * time * time * time * time * time + this.begin;
    return this.change / 2 * ((time -= 2) * time * time * time * time + 2) + this.begin;
}

WebVfx.Easing.Sinusoidal = function (begin, change, duration) {
    this.begin = begin;
    this.change = change;
    this.duration = duration;
}

WebVfx.Easing.Sinusoidal.prototype.easeIn = function (time) {
    return -this.change * Math.cos(time / this.duration * (Math.PI/2)) +
        this.change + this.begin;
}

WebVfx.Easing.Sinusoidal.prototype.easeOut = function (time) {
    return this.change * Math.sin(time / this.duration * (Math.PI/2)) +
        this.begin;
}

WebVfx.Easing.Sinusoidal.prototype.easeInOut = function (time) {
    return -this.change / 2 * (Math.cos(Math.PI * time / this.duration) - 1) +
        this.begin;
}

WebVfx.Easing.Exponential = function (begin, change, duration) {
    this.begin = begin;
    this.change = change;
    this.duration = duration;
}

WebVfx.Easing.Exponential.prototype.easeIn = function (time) {
    return (time == 0)
        ? this.begin
        : this.change * Math.pow(2, 10 * (time / this.duration - 1)) + this.begin;
}

WebVfx.Easing.Exponential.prototype.easeOut = function (time) {
    return (time == this.duration)
        ? this.begin + this.change
        : this.change * (-Math.pow(2, -10 * time / this.duration) + 1) + this.begin;
}

WebVfx.Easing.Exponential.prototype.easeInOut = function (time) {
    if (time == 0)
        return this.begin;
    if (time == this.duration)
        return this.begin + this.change;

    if ((time /= this.duration / 2) < 1)
        return this.change / 2 * Math.pow(2, 10 * (time - 1)) + this.begin;
    return this.change / 2 * (-Math.pow(2, -10 * (time - 1)) + 2) + this.begin;
}

WebVfx.Easing.Circular = function (begin, change, duration) {
    this.begin = begin;
    this.change = change;
    this.duration = duration;
}

WebVfx.Easing.Circular.prototype.easeIn = function (time) {
    return -this.change * (Math.sqrt(1 - (time /= this.duration) * time) - 1) + this.begin;
}

WebVfx.Easing.Circular.prototype.easeOut = function (time) {
    return this.change * Math.sqrt(1 - (time = time / this.duration - 1) * time) + this.begin;
}

WebVfx.Easing.Circular.prototype.easeInOut = function (time) {
    if ((time /= this.duration / 2) < 1)
        return -this.change / 2 * (Math.sqrt(1 - time * time) - 1) + this.begin;
    return this.change / 2 * (Math.sqrt(1 - (time -= 2) * time) + 1) + this.begin;
}

WebVfx.Easing.Elastic = function (begin, change, duration, amplitude, period) {
    this.begin = begin;
    this.change = change;
    this.duration = duration;
    this.amplitude = amplitude;
    this.period = period;
}

WebVfx.Easing.Elastic.prototype.easeIn = function (time) {
    if (time == 0)
        return this.begin;
    if ((time /= this.duration) == 1)
        return this.begin + this.change;
    var p = this.period;
    if (!p)
        p = this.duration * 0.3;
    var a = this.amplitude;
    var s;
    if (!a || a < Math.abs(this.change)) {
        a = this.change;
        s = p / 4;
    }
    else
        s = p / (2 * Math.PI) * Math.asin(this.change / a);
    return -(a * Math.pow(2, 10 * (time -= 1)) * Math.sin((time * this.duration - s) * (2 * Math.PI) / p)) + this.begin;
}

WebVfx.Easing.Elastic.prototype.easeOut = function (time) {
    if (time == 0)
        return this.begin;
    if ((time /= this.duration) == 1)
        return this.begin + this.change;
    var p = this.period;
    if (!p)
        p = this.duration * 0.3;
    var a = this.amplitude;
    var s;
    if (!a || a < Math.abs(this.change)) {
        a = this.change;
        s = p / 4;
    }
    else
        s = p / (2 * Math.PI) * Math.asin(this.change / a);
    return (a * Math.pow(2, -10 * time) * Math.sin((time * this.duration - s) * (2 * Math.PI) / p) + this.change + this.begin);
}

WebVfx.Easing.Elastic.prototype.easeInOut = function (time) {
    if (time == 0)
        return this.begin;
    if ((time /= this.duration / 2) == 2)
        return this.begin + this.change;
    var p = this.period;
    if (!p)
        p = this.duration * (0.3 * 1.5);
    var a = this.amplitude;
    var s;
    if (!a || a < Math.abs(this.change)) {
        a = this.change;
        s = p / 4;
    }
    else
        s = p / (2 * Math.PI) * Math.asin(this.change / a);
    if (time < 1)
        return -0.5 * (a * Math.pow(2, 10 * (time -= 1)) * Math.sin((time * this.duration - s) * (2 * Math.PI) / p)) + this.begin;
    return a * Math.pow(2, -10 * (time -= 1)) * Math.sin((time * this.duration - s) * (2 * Math.PI) / p) * 0.5 + this.change + this.begin;
}

WebVfx.Easing.Back = function (begin, change, duration, overshoot) {
    this.begin = begin;
    this.change = change;
    this.duration = duration;
    this.overshoot = overshoot ? overshoot : 1.70158;
}

WebVfx.Easing.Back.prototype.easeIn = function (time) {
    return this.change * (time /= this.duration) * time * ((this.overshoot + 1) * time - this.overshoot) + this.begin;
}

WebVfx.Easing.Back.prototype.easeOut = function (time) {
    return this.change * ((time = time / this.duration - 1) * time * ((this.overshoot + 1) * time + this.overshoot) + 1) + this.begin;
}

WebVfx.Easing.Back.prototype.easeInOut = function (time) {
    var s = this.overshoot;
    if ((time /= this.duration / 2) < 1)
        return this.change / 2 * (time * time * (((s *= (1.525)) + 1) * time - s)) + this.begin;
    return this.change / 2 * ((time -= 2) * time * (((s *= (1.525)) + 1) * time + s) + 2) + this.begin;
}

WebVfx.Easing.Bounce = function (begin, change, duration) {
    this.begin = begin;
    this.change = change;
    this.duration = duration;
}

WebVfx.Easing.Bounce.prototype.easeInternal = function (time, begin, change, duration) {
    if ((time /= duration) < (1 / 2.75))
        return change * (7.5625 * time * time) + begin;
    else if (time < (2 / 2.75))
        return change * (7.5625 * (time -= (1.5 / 2.75)) * time + 0.75) + begin;
    else if (time < (2.5 / 2.75))
        return change * (7.5625 * (time -= (2.25 / 2.75)) * time + 0.9375) + begin;
    else
        return change * (7.5625 * (time -= (2.625 / 2.75)) * time + 0.984375) + begin;
}

WebVfx.Easing.Bounce.prototype.easeIn = function (time) {
    return this.change - this.easeInternal(this.duration - time, 0, this.change, this.duration) + this.begin;
}

WebVfx.Easing.Bounce.prototype.easeOut = function (time) {
    return this.easeInternal(time, this.begin, this.change, this.duration);
}

WebVfx.Easing.Bounce.prototype.easeInOut = function (time) {
    if (time < this.duration / 2)
        return (this.change - this.easeInternal(this.duration - time * 2, 0, this.change, this.duration)) * 0.5 + this.begin;
    else
        return this.easeInternal(time * 2 - this.duration, 0, this.change, this.duration) * 0.5 + this.change * 0.5 + this.begin;
}
