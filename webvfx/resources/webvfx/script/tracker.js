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

// _trackedFont_ is the font name used with @font-face
// _fallbackFont_ is a builtin font with a different size than _trackedFont_
//  (e.g. "monospace")
// _callback_ will be called before _tracker_ is decremented - so it
//  can add additional tracking if needed
WebVfx.FontTracker = function (trackedFont, fallbackFont, tracker, callback) {
    // Quote both names in case they have spaces
    fallbackFont = this.quote(fallbackFont);
    trackedFont = this.quote(trackedFont);

    var trackedSpan = this.createSpan(trackedFont + "," + fallbackFont);
    var fallbackSpan = this.createSpan(fallbackFont);
    if (!this.isFontLoaded(trackedSpan, fallbackSpan)) {
        var self = this;
        tracker.increment();
        var intervalID = setInterval(function () {
            if (self.isFontLoaded(trackedSpan, fallbackSpan)) {
                clearInterval(intervalID);
                callback();
                tracker.decrement();
            }
        }, 100);
    }
    else
        callback();
}

WebVfx.FontTracker.prototype = {
    createSpan: function (fontFamily) {
        var styleString = "position:absolute; visibility:hidden; font-size:300px; width:auto; height:auto; line-height:normal; margin:0; padding:0; font-variant:normal; font-family:";
        var span = document.createElement("span");
        span.style.cssText = styleString + fontFamily;
        span.textContent = "BESs";
        document.body.appendChild(span);
        return span;
    },
    quote: function (name) {
        if (name[0] != '"' && name[0] != "'")
            return "'" + name + "'";
        return name;
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