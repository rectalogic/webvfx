TEMPLATE = app
INCLUDEPATH += ..
DESTDIR = ../build

SOURCES = color_swatch.cpp main.cpp viewer.cpp
HEADERS = color_swatch.h viewer.h
FORMS = viewer.ui

CONFIG += warn_on debug_and_release
QT += webkit
CONFIG(debug, debug|release) {
    TARGET = viewer_debug
    LIBS += -L$$DESTDIR -lwebvfx_debug
    UI_DIR = debug
} else {
    TARGET = viewer
    LIBS += -L$$DESTDIR -lwebvfx
    UI_DIR = release
}
