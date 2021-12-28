# Copyright (c) 2011 Hewlett-Packard Development Company, L.P. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

include(../common.pri)
TEMPLATE = lib

HEADERS += factory.h
HEADERS += service_locker.h
HEADERS += service_manager.h

SOURCES += factory.cpp
SOURCES += service_locker.cpp
SOURCES += service_manager.cpp
SOURCES += webvfx_filter.cpp
SOURCES += webvfx_producer.cpp
SOURCES += webvfx_transition.cpp

CONFIG += plugin shared

CONFIG += link_pkgconfig
PKGCONFIG += mlt-framework-7

TARGET = mltwebvfx

DEPENDPATH += $$PWD/../webvfx
CONFIG(release, debug|release): LIBS += -L$$OUT_PWD/../webvfx/release/ -lwebvfx
else:CONFIG(debug, debug|release): LIBS += -L$$OUT_PWD/../webvfx/debug/ -lwebvfx

QMAKE_RPATHDIR += $$[QT_INSTALL_LIBS]

linux:QMAKE_LFLAGS += -Wl,-znodelete

# Install in mlt plugins directory
target.path = $$system(pkg-config --variable=libdir mlt-framework-7)/mlt
INSTALLS += target
