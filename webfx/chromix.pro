TEMPLATE = lib
#VERSION = 

HEADERS += chromix.h
HEADERS += logger.h
HEADERS += mix_kit.h
HEADERS += web_page.h
HEADERS += web_renderer.h
HEADERS += web_script.h

SOURCES += chromix.cpp
SOURCES += logger.cpp
SOURCES += mix_kit.cpp
SOURCES += web_page.cpp
SOURCES += web_renderer.cpp
SOURCES += web_script.cpp

INCLUDEPATH = ..

DESTDIR = ../build

CONFIG += qt dll thread warn_on
#XXX add opengl?
QT += webkit

CONFIG += debug_and_release
CONFIG(debug, debug|release) {
    TARGET = chromix
} else {
    TARGET = chromix
}
