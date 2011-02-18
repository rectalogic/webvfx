TEMPLATE = app
INCLUDEPATH += ..
DESTDIR = ../build

SOURCES = main.cpp image_color.cpp viewer.cpp
HEADERS = image_color.h viewer.h
FORMS = viewer.ui

CONFIG += warn_on debug_and_release
QT += webkit declarative
CONFIG(debug, debug|release) {
    !mac:TARGET = viewer_debug
    mac:TARGET = Viewer_debug
    LIBS += -L$$DESTDIR -lwebvfx_debug
    UI_DIR = debug
} else {
    !mac:TARGET = viewer
    mac:TARGET = Viewer
    LIBS += -L$$DESTDIR -lwebvfx
    UI_DIR = release
}
