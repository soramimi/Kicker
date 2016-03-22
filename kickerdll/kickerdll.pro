TEMPLATE = lib
CONFIG += dll c++11
CONFIG -= app_bundle
CONFIG -= qt

TARGET=kicker

DEF_FILE=kicker.def

SOURCES += \
    kicker.cpp

HEADERS += \
    common.h \
    KickerAPI.h \
    resource.h

LIBS += gdi32.lib
LIBS += user32.lib

Debug:DLLDESTDIR=$$PWD/../build/debug
Release:DLLDESTDIR=$$PWD/../build/release


