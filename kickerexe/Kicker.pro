#-------------------------------------------------
#
# Project created by QtCreator 2014-10-28T11:16:13
#
#-------------------------------------------------

QT       += core gui xml widgets

TARGET = kicker
TEMPLATE = app

TRANSLATIONS = kicker_ja.ts

SOURCES += src/main.cpp\
        src/MainWindow.cpp \
    src/win.cpp \
    misc.cpp \
    src/MySettings.cpp \
    src/pathcat.cpp \
    src/Options.cpp \
    src/ShellExecuteDialog.cpp \
    src/DecoyWindow.cpp

HEADERS  += src/MainWindow.h \
	src/win.h \
    misc.h \
    src/MySettings.h \
    src/pathcat.h \
    src/Options.h \
    src/ShellExecuteDialog.h \
    src/DecoyWindow.h \
    src/main.h

FORMS    += src/MainWindow.ui \
    src/ShellExecuteDialog.ui \
    src/DecoyWindow.ui

RESOURCES += \
    resources.qrc

win32:RC_FILE = Kicker.rc
win32:LIBS += user32.lib

Debug:DESTDIR=$$PWD/../build/debug
Release:DESTDIR=$$PWD/../build/release
