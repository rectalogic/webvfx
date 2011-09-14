var WebVfx = WebVfx || {};

// Generic resource tracker - calls _callback_ when count reaches 0
WebVfx.Tracker = function (callback) {
    this.callback = callback;
    this.count = 0;
    this.started = false;
};

WebVfx.Tracker.prototype = {
    increment: function () {
        this.count++;
    },
    decrement: function () {
        if (--this.count <= 0 && this.started)
            this.callback();
    },
    start: function () {
        if (this.count <= 0)
            this.callback();
        else
            this.started = true;
    }
};

///////////

// Waits for a font specified by @font-face to be loaded and useable.
// Otherwise drawing to a canvas draws nothing:
// http://code.google.com/p/chromium/issues/detail?id=32879
// And measuring text will return the wrong size:
// http://stackoverflow.com/questions/3311005/jquery-working-out-wrong-height-due-to-font-face
// Based on Google WebFont Loader:
// https://github.com/typekit/webfontloader/blob/master/src/core/fontwatchrunner.js
// This currently has issues with QtWebKit:
// https://bugs.webkit.org/show_bug.cgi?id=55036

// _trackedFont_ is the font name used with @font-face
// _fallbackFont_ is a builtin font with different metrics than _trackedFont_
//  (e.g. "monospace")
// _callback_ will be called before _tracker_ is decremented - so it
//  can add additional tracking if needed
WebVfx.FontTracker = function (trackedFont, fallbackFont, tracker, callback) {
    var trackedSpan = this.createSpan(trackedFont + "," + fallbackFont);
    var fallbackSpan = this.createSpan(fallbackFont);
    if (!this.isFontLoaded(trackedSpan, fallbackSpan)) {
        var self = this;
        tracker.increment();
        var intervalID = setInterval(function () {
            if (self.isFontLoaded(trackedSpan, fallbackSpan)) {
                clearInterval(intervalID);
                if (callback) callback();
                tracker.decrement();
            }
        }, 25);
    }
    else if (callback)
        callback();
}

WebVfx.FontTracker.prototype = {
    createSpan: function (fontFamily) {
        var styleString = "position:absolute; visibility:hidden; font-size:300px; width:auto; height:auto; line-height:normal; margin:0; padding:0; font-variant:normal; font-family:";
        var span = document.createElement("span");
        span.style.cssText = styleString + fontFamily;
        span.textContent = "BESIsi";
        document.body.appendChild(span);
        return span;
    },
    isFontLoaded: function (trackedSpan, fallbackSpan) {
        if (trackedSpan.offsetWidth != fallbackSpan.offsetWidth ||
            trackedSpan.offsetHeight != fallbackSpan.offsetHeight) {
            document.body.removeChild(trackedSpan);
            document.body.removeChild(fallbackSpan);
            return true;
        }
        return false;
    }
}