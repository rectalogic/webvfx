# Copyright (c) 2011 Hewlett-Packard Development Company, L.P. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

include(../../common.pri)
TEMPLATE = app

SOURCES += render.cpp

CONFIG += console
mac:CONFIG -= app_bundle
TARGET = webvfx_render

DEPENDPATH += $$PWD/../../webvfx
CONFIG(release, debug|release): LIBS += -L$$OUT_PWD/../../webvfx/release/ -lwebvfx
else:CONFIG(debug, debug|release): LIBS += -L$$OUT_PWD/../../webvfx/debug/ -lwebvfx

QMAKE_RPATHDIR += $$[QT_INSTALL_LIBS]

target.path = $$[QT_INSTALL_BINS]
INSTALLS += target
