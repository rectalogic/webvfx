# Copyright (c) 2011 Hewlett-Packard Development Company, L.P. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

include(../../common.pri)
TEMPLATE = app

SOURCES += render.cpp

CONFIG += console
mac:CONFIG -= app_bundle
TARGET = webvfx_render
LIBS += -L$$DESTDIR -lwebvfx

QMAKE_RPATHDIR += $$PREFIX/lib

target.path = $$PREFIX/bin
INSTALLS += target
