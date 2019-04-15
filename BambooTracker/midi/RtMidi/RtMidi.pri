SOURCES += \
    $$PWD/RtMidi.cpp

HEADERS += \
    $$PWD/RtMidi.h

linux {
    DEFINES += __LINUX_ALSA__
    LIBS += -lasound
}
win32 {
    DEFINES += __WINDOWS_MM__
    LIBS += -lwinmm
}
macx {
    DEFINES += __MACOSX_CORE__
    LIBS += -framework CoreMIDI -framework CoreAudio -framework CoreFoundation
}
