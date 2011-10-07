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

    imageMap: {}, // User can set this to map image names to Images
    parameterMap: {}, // User can set this to map param names to strings/numbers

    readyRender: function (v) {},
    getImage: function (name) {
        var imageMap = this.imageMap;
        return {
            toImageData: function () {
                var image = imageMap[name];
                var canvas = document.createElement("canvas");
                if (image) {
                    canvas.width = image.width;
                    canvas.height = image.height;
                }
                else
                    canvas.width = canvas.height = 320;
                var context = canvas.getContext('2d');
                if (image)
                    context.drawImage(image, 0, 0);
                else {
                    context.fillStyle = 'rgb(255,0,0)';
                    context.fillRect(0, 0, canvas.width, canvas.height);
                }
                return context.getImageData(0, 0, canvas.width, canvas.height);
            },
            assignToHTMLImageElement: function (image) {
                if (imageMap[name])
                    image.src = imageMap[name].src;
            }
        };
    },
    getStringParameter: function (name) {
        return this.parameterMap[name];
    },
    getNumberParameter: function (name) {
        return parseFloat(this.parameterMap[name]);
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