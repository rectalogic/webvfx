TEMPLATE = app
INCLUDEPATH += ..
DESTDIR = ../build

SOURCES += main.cc

CONFIG += warn_on debug_and_release
CONFIG(debug, debug|release) {
    TARGET = demo_debug
    LIBS += -L$$DESTDIR -lwebfx_debug
} else {
    TARGET = demo
    LIBS += -L$$DESTDIR -lwebfx
}
