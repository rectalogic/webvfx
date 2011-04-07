include(../../common.pri)
TEMPLATE = app

SOURCES = main.cpp browser.cpp
HEADERS = browser.h
FORMS = browser.ui

QT += webkit
!mac:TARGET = qtbrowser
mac:TARGET = QtBrowser