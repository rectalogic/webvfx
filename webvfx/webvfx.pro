include(../common.pri)
TEMPLATE = lib
#VERSION = 

HEADERS += content.h
HEADERS += content_context.h
HEADERS += effects.h
HEADERS += effects_impl.h
HEADERS += image.h
HEADERS += logger.h
HEADERS += parameters.h
HEADERS += qml_content.h
HEADERS += web_content.h
HEADERS += webvfx.h

SOURCES += content.cpp
SOURCES += content_context.cpp
SOURCES += effects.cpp
SOURCES += effects_impl.cpp
SOURCES += image.cpp
SOURCES += logger.cpp
SOURCES += parameters.cpp
SOURCES += qml_content.cpp
SOURCES += web_content.cpp
SOURCES += webvfx.cpp
macx:SOURCES += webvfx_mac.mm

RESOURCES += resources/resources.qrc

macx:LIBS += -framework Foundation

CONFIG += shared thread
QT += webkit opengl declarative

TARGET = webvfx

target.path = $$PREFIX/lib
INSTALLS += target