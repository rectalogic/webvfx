include(../common.pri)
TEMPLATE = app

SOURCES += main.cpp

CONFIG += console
QT -= gui
TARGET = webvfx_demo
LIBS += -L$$DESTDIR -lwebvfx

QMAKE_RPATHDIR += $$PREFIX/lib
