TEMPLATE = lib
#VERSION = 

HEADERS += logger.h
HEADERS += web_effects.h
HEADERS += web_page.h
HEADERS += web_renderer.h
HEADERS += web_script.h
HEADERS += webfx.h

SOURCES += logger.cpp
SOURCES += web_effects.cpp
SOURCES += web_page.cpp
SOURCES += web_renderer.cpp
SOURCES += web_script.cpp
SOURCES += webfx.cpp

INCLUDEPATH = ..

DESTDIR = ../build

CONFIG += qt dll thread warn_on
#XXX add opengl?
QT += webkit

CONFIG += debug_and_release
CONFIG(debug, debug|release) {
    TARGET = webfx
} else {
    TARGET = webfx
}
