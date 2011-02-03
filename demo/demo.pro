TEMPLATE = app
INCLUDEPATH += ..
DESTDIR = ../build

SOURCES += main.cpp

CONFIG += console warn_on debug_and_release
CONFIG -= qt
CONFIG(debug, debug|release) {
    TARGET = demo_debug
    LIBS += -L$$DESTDIR -lwebfx_debug
} else {
    TARGET = demo
    LIBS += -L$$DESTDIR -lwebfx
}
