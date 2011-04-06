isEmpty(PREFIX) {
    unix:PREFIX = /usr/local
}

CONFIG += warn_on debug_and_release

CONFIG(debug, debug|release) {
    DESTDIR = $$PWD/build/debug
} else {
    DESTDIR = $$PWD/build/release
}

OBJECTS_DIR = $$DESTDIR/.obj/$$TARGET
MOC_DIR = $$DESTDIR/.moc/$$TARGET
RCC_DIR = $$DESTDIR/.rcc/$$TARGET
UI_DIR = $$DESTDIR/.ui/$$TARGET

INCLUDEPATH += $$PWD
