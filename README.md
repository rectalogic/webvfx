# Overview

WebVfx is a video effects library that allows effects to be implemented using [WebKit HTML](http://trac.webkit.org/wiki/QtWebKit) or [Qt QML](http://qt-project.org/doc/qt-5/qmlapplications.html). It is being developed for MacOS and Linux.

## Prerequisites

You will need [Qt](http://qt-project.org/downloads/) installed (4.7.3 or later recommended). Qt includes QtWebKit and QML. For more interesting effects (e.g. using WebGL, CSS 3D etc.) you may want to [build](http://trac.webkit.org/wiki/QtWebKit#BuildInstructions) a more current QtWebKit.
You may also want to build [Qt3D](http://qt-project.org/wiki/Qt3D-Installation) - 3D extensions to QML. WebVfx includes support for Qt3D.

WebVfx includes an MLT service that exposes producers, filters and transitions implemented in WebVfx. Install [MLT Framework](http://www.mltframework.org/) 0.7.2 or greater to build the plugin.

## Building

In the webvfx directory run `qmake -r PREFIX=/usr/local` and then `make install`. `PREFIX` determines where WebVfx will be installed. If MLT is installed in a non-standard location, you may need to set the `PKG_CONFIG_PATH` environment variable to where its pkgconfig file lives, e.g. `PKG_CONFIG_PATH=/usr/local/lib/pkgconfig`.

The [MLT melt](http://www.mltframework.org/twiki/bin/view/MLT/MltMelt) command will not work with WebVfx on Windows or OS X because the Qt event loop must run on the main thread. If you set `MLT_SOURCE` to the root of your MLT source code directory, then a `qmelt` executable will be installed which behaves the same as `melt` but works with WebVfx on Windows or OS X. e.g. `qmake -r PREFIX=/usr/local MLT_SOURCE=~/Projects/mlt`.

`make doxydoc` to generate the documentation using Doxygen.
You can also `make uninstall`, `make clean` and `make distclean`.

## Demos

See the [documentation](http://www.mltframework.org/doxygen/webvfx/) for examples.

## License

Copyright (c) 2011 Hewlett-Packard Development Company, L.P. All rights reserved.
Use of this source code is governed by a BSD-style license that can be
found in the LICENSE file.
