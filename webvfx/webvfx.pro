# Copyright (c) 2011 Hewlett-Packard Development Company, L.P. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

include(../common.pri)
TEMPLATE = lib
#VERSION = 

HEADERS += content.h
HEADERS += content_context.h
HEADERS += effects.h
HEADERS += effects_impl.h
HEADERS += image.h
HEADERS += logger.h
HEADERS += parameters.h
HEADERS += qml_content.h
HEADERS += webvfx.h

SOURCES += content.cpp
SOURCES += content_context.cpp
SOURCES += effects.cpp
SOURCES += effects_impl.cpp
SOURCES += image.cpp
SOURCES += logger.cpp
SOURCES += parameters.cpp
SOURCES += qml_content.cpp
SOURCES += webvfx.cpp

RESOURCES += resources/resources.qrc

CONFIG += shared thread qmltypes
QT += core gui gui-private qml quick quick-private quick3d
macx:QTPLUGIN.platforms += qoffscreen

QML_IMPORT_NAME = org.webvfx.WebVfx.native
QML_IMPORT_MAJOR_VERSION = 1

TARGET = webvfx

target.path = $$[QT_INSTALL_LIBS]
INSTALLS += target

macx:QMAKE_SONAME_PREFIX = @rpath