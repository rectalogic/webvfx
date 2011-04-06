include(common.pri)

TEMPLATE = subdirs
CONFIG += ordered

SUBDIRS += webvfx
SUBDIRS += viewer
SUBDIRS += demo

unix:system(pkg-config --exists mlt-framework) {
    SUBDIRS += mlt
    mlt.depends = webvfx
} else {
    message("MLT framework not found, skipping MLT plugin. Need to set PKG_CONFIG_PATH?")
}

viewer.depends = webvfx
demo.depends = webvfx
