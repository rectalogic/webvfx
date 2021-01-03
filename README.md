# Overview

WebVfx is a video effects library that allows visual effects to be implemented using [QtQuick QML](https://doc.qt.io/qt-6/qtquick-index.html). It is being developed for MacOS and Linux.

## Prerequisites

You will need [Qt](https://www.qt.io/download) installed (6.0 or later required). Qt includes QtQuick and QtQuick3D.

WebVfx includes an MLT service that exposes producers, filters and transitions implemented in WebVfx. Install [MLT Framework](http://www.mltframework.org/) 0.7.2 or greater to build the plugin.

## Building

In the webvfx root directory create a shadow build subdirectory then build and install:
```sh-session
$ mkdir build
$ cd build
$ qmake ..
$ make install
```
If MLT is installed in a non-standard location, you may need to set the `PKG_CONFIG_PATH` environment variable to where its pkgconfig file lives, e.g. `PKG_CONFIG_PATH=/usr/local/lib/pkgconfig`.

`make doxydoc` to generate the documentation using Doxygen.
You can also `make uninstall`, `make clean` and `make distclean`.

## Demos

See the [documentation](http://rectalogic.github.com/webvfx/) for examples.

## License

Copyright (c) 2011 Hewlett-Packard Development Company, L.P. All rights reserved.
Use of this source code is governed by a BSD-style license that can be
found in the LICENSE file.
