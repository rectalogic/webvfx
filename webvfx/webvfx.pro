TEMPLATE = lib
#VERSION = 

HEADERS += effects.h
HEADERS += image.h
HEADERS += logger.h
HEADERS += parameters.h
HEADERS += rendering_context.h
HEADERS += web_page.h
HEADERS += web_renderer.h
HEADERS += webvfx.h

SOURCES += effects.cpp
SOURCES += image.cpp
SOURCES += logger.cpp
SOURCES += parameters.cpp
SOURCES += rendering_context.cpp
SOURCES += web_page.cpp
SOURCES += web_renderer.cpp
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
