include(../common.pri)
TEMPLATE = app

SOURCES = main.cpp image_color.cpp viewer.cpp
HEADERS = image_color.h viewer.h
FORMS = viewer.ui

QT += webkit declarative
!mac:TARGET = webvfx_viewer
mac:TARGET = "WebVfx Viewer"
LIBS += -L$$DESTDIR -lwebvfx

QMAKE_RPATHDIR += $$PREFIX/lib

target.path = $$PREFIX/bin
INSTALLS += target
