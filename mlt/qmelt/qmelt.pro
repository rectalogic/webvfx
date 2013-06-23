isEmpty(MLT_SOURCE) {
    error("qmelt can only be built when MLT_SOURCE is set to the MLT source code directory.")
}

include(../../common.pri)
TEMPLATE = app

SOURCES += qmelt.cpp
SOURCES += $$MLT_SOURCE/src/melt/melt.c
SOURCES += $$MLT_SOURCE/src/melt/io.c

DEFINES += main=melt_main
DEFINES += VERSION=\\\"qmelt\\\"

INCLUDEPATH += $$MLT_SOURCE

CONFIG -= app_bundle

CONFIG += link_pkgconfig
PKGCONFIG += mlt-framework

win32 {
    CONFIG += console
    DEFINES += MELT_NOSDL
}

TARGET = qmelt
QMAKE_RPATHDIR += $$PREFIX/lib

target.path = $$PREFIX/bin
INSTALLS += target
