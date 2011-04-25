# Copyright (c) 2011 Hewlett-Packard Development Company, L.P. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

include(../../common.pri)
TEMPLATE = app

SOURCES = main.cpp browser.cpp
HEADERS = browser.h
FORMS = browser.ui

QT += webkit
!mac:TARGET = webvfx_browser
mac:TARGET = "WebVfx Browser"

target.path = $$PREFIX/bin
INSTALLS += target
