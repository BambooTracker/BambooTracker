#-------------------------------------------------
#
# Project created by QtCreator 2018-06-09T16:20:11
#
#-------------------------------------------------

QT       += core gui multimedia

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = BambooTracker
TEMPLATE = app

# The following define makes your compiler emit warnings if you use
# any feature of Qt which has been marked as deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

# You can also make your code fail to compile if you use deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

CONFIG += c++14

SOURCES += \
        main.cpp \
        mainwindow.cpp \
    chips/chip.cpp \
    chips/opna.cpp \
    chips/resampler.cpp \
    chips/mame/2608intf.c \
    chips/mame/emu2149.c \
    chips/mame/fm.c \
    chips/mame/ymdeltat.c \
    bamboo_tracker.cpp \
    stream/audio_stream.cpp \
    stream/audio_stream_mixier.cpp

HEADERS += \
        mainwindow.hpp \
    chips/mame/2608intf.h \
    chips/mame/emu2149.h \
    chips/mame/emutypes.h \
    chips/mame/fm.h \
    chips/mame/mamedef.h \
    chips/mame/ymdeltat.h \
    chips/chip.hpp \
    chips/chip_misc.h \
    chips/opna.hpp \
    chips/resampler.hpp \
    bamboo_tracker.hpp \
    stream/audio_stream.hpp \
    stream/audio_stream_mixier.hpp \
    chips/chip_def.h

FORMS += \
        mainwindow.ui

INCLUDEPATH += \
    $$PWD/chips \
    $$PWD/stream
