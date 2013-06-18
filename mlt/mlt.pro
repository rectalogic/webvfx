# Copyright (c) 2011 Hewlett-Packard Development Company, L.P. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

include(../common.pri)
TEMPLATE = lib

HEADERS += factory.h
HEADERS += service_locker.h
HEADERS += service_manager.h

SOURCES += factory.cpp
SOURCES += panzoom_producer.cpp
SOURCES += service_locker.cpp
SOURCES += service_manager.cpp
SOURCES += webvfx_filter.cpp
SOURCES += webvfx_producer.cpp
SOURCES += webvfx_transition.cpp

CONFIG += plugin shared

CONFIG += link_pkgconfig
PKGCONFIG += mlt-framework

TARGET = mltwebvfx

LIBS += -L$$DESTDIR -lwebvfx

win32 {
    QT += webkit opengl declarative
    LIBS += -lglu32 -lopengl32 -lpthread
}

QMAKE_RPATHDIR += $$PREFIX/lib

# Install in mlt plugins directory
target.path = $$system(pkg-config --variable=libdir mlt-framework)/mlt
INSTALLS += target
# Add mlt plugins to rpath so we can dlopen ourself without a full path.
QMAKE_RPATHDIR += $$target.path
