!mac|isEmpty(MLT_SOURCE) {
    error("qmelt should only be built on MacOS with MLT_SOURCE defined")
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
unix {
    CONFIG += link_pkgconfig
    PKGCONFIG += mlt-framework
}

TARGET = qmelt
QMAKE_RPATHDIR += $$PREFIX/lib

target.path = $$PREFIX/bin
INSTALLS += target
