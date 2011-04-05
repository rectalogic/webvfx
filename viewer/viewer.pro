include(../common.pri)
TEMPLATE = app

SOURCES = main.cpp image_color.cpp viewer.cpp
HEADERS = image_color.h viewer.h
FORMS = viewer.ui

CONFIG += warn_on debug_and_release
QT += webkit declarative
CONFIG(debug, debug|release) {
    !mac:TARGET = webvfx_viewer_debug
    mac:TARGET = "WebVfx Viewer debug"
    LIBS += -L$$DESTDIR -lwebvfx_debug
    UI_DIR = debug
} else {
    !mac:TARGET = webvfx_viewer
    mac:TARGET = "WebVfx Viewer"
    LIBS += -L$$DESTDIR -lwebvfx
    UI_DIR = release
}

QMAKE_RPATHDIR += $$PREFIX/lib

target.path = $$PREFIX/bin
INSTALLS += target
