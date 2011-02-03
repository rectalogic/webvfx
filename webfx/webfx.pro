TEMPLATE = lib
#VERSION = 

HEADERS += web_effects.h
HEADERS += web_image.h
HEADERS += web_logger.h
HEADERS += web_page.h
HEADERS += web_parameters.h
HEADERS += web_renderer.h
HEADERS += web_script.h
HEADERS += webfx.h

SOURCES += web_effects.cpp
HEADERS += web_image.h
SOURCES += web_logger.cpp
SOURCES += web_page.cpp
SOURCES += web_parameters.cpp
SOURCES += web_renderer.cpp
SOURCES += web_script.cpp
SOURCES += webfx.cpp
mac:SOURCES += webfx_mac.mm

mac:LIBS += -framework Foundation

INCLUDEPATH = ..

DESTDIR = ../build

CONFIG += shared thread warn_on
#XXX add opengl?
QT += webkit


CONFIG += debug_and_release
CONFIG(debug, debug|release) {
    TARGET = webfx_debug
} else {
    TARGET = webfx
}
