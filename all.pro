TEMPLATE = subdirs
CONFIG += ordered debug_and_release

SUBDIRS += webvfx
SUBDIRS += viewer
SUBDIRS += demo

unix:system(pkg-config --exists --print-errors mlt-framework) {
    SUBDIRS += mlt
    mlt.depends = webvfx
} else {
    message("MLT framework not found, skipping MLT plugin")
}

viewer.depends = webvfx
demo.depends = webvfx
