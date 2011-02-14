TEMPLATE = lib
#VERSION = 

HEADERS += effects.h
HEADERS += effects_context.h
HEADERS += image.h
HEADERS += logger.h
HEADERS += parameters.h
HEADERS += web_effects.h
HEADERS += web_page.h
HEADERS += webvfx.h

SOURCES += effects.cpp
SOURCES += effects_context.cpp
SOURCES += image.cpp
SOURCES += logger.cpp
SOURCES += parameters.cpp
SOURCES += web_effects.cpp
SOURCES += web_page.cpp
SOURCES += webvfx.cpp
macx:SOURCES += webvfx_mac.mm

macx:LIBS += -framework Foundation

INCLUDEPATH = ..

DESTDIR = ../build

CONFIG += shared thread warn_on
#XXX add opengl?
QT += webkit


CONFIG += debug_and_release
CONFIG(debug, debug|release) {
    TARGET = webvfx_debug
} else {
    TARGET = webvfx
}
