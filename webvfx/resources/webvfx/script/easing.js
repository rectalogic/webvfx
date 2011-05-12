// Copyright (c) 2011 Hewlett-Packard Development Company, L.P. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

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
