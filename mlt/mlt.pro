# Copyright (c) 2011 Hewlett-Packard Development Company, L.P. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

include(../common.pri)
TEMPLATE = lib

HEADERS += service_locker.h
HEADERS += service_manager.h
HEADERS += webvfx_service.h

SOURCES += factory.cpp
SOURCES += service_locker.cpp
SOURCES += service_manager.cpp
SOURCES += webvfx_filter.cpp
SOURCES += webvfx_producer.cpp
SOURCES += webvfx_service.cpp
SOURCES += webvfx_transition.cpp

CONFIG += plugin shared
QT -= gui

unix {
    CONFIG += link_pkgconfig
    PKGCONFIG += mlt-framework
}

TARGET = mltwebvfx
LIBS += -L$$DESTDIR -lwebvfx

QMAKE_RPATHDIR += $$PREFIX/lib

unix {
    # Install in mlt plugins directory
    target.path = $$system(pkg-config --variable=libdir mlt-framework)/mlt
    INSTALLS += target

    # Create directory to hold qml/html effects
    data.path = $$system(pkg-config --variable=datadir mlt-framework)/mlt/webvfx
    data.extra = mkdir -p $$data.path
    INSTALLS += data
}
