TEMPLATE = lib
#VERSION = 

HEADERS += content.h
HEADERS += effects.h
HEADERS += effects_context.h
HEADERS += effects_impl.h
HEADERS += image.h
HEADERS += logger.h
HEADERS += parameters.h
HEADERS += qml_content.h
HEADERS += web_content.h
HEADERS += webvfx.h

SOURCES += content.cpp
SOURCES += effects.cpp
SOURCES += effects_context.cpp
SOURCES += effects_impl.cpp
SOURCES += image.cpp
SOURCES += logger.cpp
SOURCES += parameters.cpp
SOURCES += qml_content.cpp
SOURCES += web_content.cpp
SOURCES += webvfx.cpp
macx:SOURCES += webvfx_mac.mm

macx:LIBS += -framework Foundation

INCLUDEPATH = ..

DESTDIR = ../build

CONFIG += shared thread warn_on
#XXX add opengl?
QT += webkit declarative


CONFIG += debug_and_release
CONFIG(debug, debug|release) {
    TARGET = webvfx_debug
} else {
    TARGET = webvfx
}
