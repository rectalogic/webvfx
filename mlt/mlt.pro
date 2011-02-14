TEMPLATE = lib
#VERSION = 

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

INCLUDEPATH = ..

DESTDIR = ../build

CONFIG += shared warn_on
QT -= gui

CONFIG += link_pkgconfig
PKGCONFIG += mlt-framework

CONFIG += debug_and_release
CONFIG(debug, debug|release) {
    TARGET = mltwebvfx_debug
    LIBS += -L$$DESTDIR -lwebvfx_debug
} else {
    TARGET = mltwebvfx
    LIBS += -L$$DESTDIR -lwebvfx
}
