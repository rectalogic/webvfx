TEMPLATE = subdirs
CONFIG += ordered debug_and_release

SUBDIRS += webvfx
SUBDIRS += viewer
SUBDIRS += demo
SUBDIRS += mlt


viewer.depends = webvfx
demo.depends = webvfx
mlt.depends = webvfx
