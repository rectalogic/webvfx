// Copyright (c) 2011 Hewlett-Packard Development Company, L.P. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Stubs for the webvfx JS context object.
// This can be included in an HTML effect so it can be tested
// in a web browser.

var webvfx = {
    TargetImageType: 1,
    SourceImageType: 2,
    ExtraImageType: 3,
    readyRender: function (v) {},
    getImage: function (name) {
        if (!this.imageData) {
            var canvas = document.createElement("canvas");
            canvas.width = canvas.height = 320;
            this.imageData = canvas.getContext('2d').createImageData(320, 320);
        }
        var imageData = this.imageData;
        return {
            toImageData: function () {
                return imageData;
            },
            assignToHTMLImageElement: function (image) {
            }
        };
    },
    getStringParameter: function (name) {
        return "webvfx";
    },
    getNumberParameter: function (name) {
        return 0;
    },
    renderRequested: {
        connect: function (f) {
            this.render = f;
        }
    },
    render: function (t) {
        this.renderRequested.render(t);
    },
};