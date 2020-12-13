# Copyright (c) 2011 Hewlett-Packard Development Company, L.P. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

!versionAtLeast(QT_VERSION, 6.0.0) {
    message("Cannot use Qt $${QT_VERSION}")
    error("Use Qt 6.0 or newer")
}

isEmpty(PREFIX) {
    unix:PREFIX = /usr/local
}

CONFIG += warn_on debug_and_release

CONFIG(debug, debug|release) {
    DESTDIR = $$PWD/build/debug
} else {
    DESTDIR = $$PWD/build/release
}

OBJECTS_DIR = $$DESTDIR/.obj/$$TARGET
MOC_DIR = $$DESTDIR/.moc/$$TARGET
RCC_DIR = $$DESTDIR/.rcc/$$TARGET
UI_DIR = $$DESTDIR/.ui/$$TARGET

INCLUDEPATH += $$PWD
