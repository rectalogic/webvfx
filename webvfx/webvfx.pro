TEMPLATE = lib
#VERSION = 

HEADERS += web_effects.h
HEADERS += image.h
HEADERS += logger.h
HEADERS += web_page.h
HEADERS += parameters.h
HEADERS += web_renderer.h
HEADERS += web_script.h
HEADERS += webvfx.h

SOURCES += web_effects.cpp
SOURCES += image.cpp
SOURCES += logger.cpp
SOURCES += web_page.cpp
SOURCES += parameters.cpp
SOURCES += web_renderer.cpp
SOURCES += web_script.cpp
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
