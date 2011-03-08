include(../common.pri)
TEMPLATE = app

SOURCES += main.cpp

CONFIG += console warn_on debug_and_release
QT -= gui
CONFIG(debug, debug|release) {
    TARGET = demo_debug
    LIBS += -L$$DESTDIR -lwebvfx_debug
} else {
    TARGET = demo
    LIBS += -L$$DESTDIR -lwebvfx
}

QMAKE_RPATHDIR += $$PREFIX/lib
