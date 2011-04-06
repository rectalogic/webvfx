include(../common.pri)
TEMPLATE = lib

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

CONFIG += plugin shared
QT -= gui

unix {
    CONFIG += link_pkgconfig
    PKGCONFIG += mlt-framework
}

TARGET = mltwebvfx
LIBS += -L$$DESTDIR -lwebvfx

QMAKE_RPATHDIR += $$PREFIX/lib

# Install in mlt plugins directory
unix:target.path = $$system(pkg-config --variable=libdir mlt-framework)/mlt
INSTALLS += target