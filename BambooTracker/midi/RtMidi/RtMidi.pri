SOURCES += \
    $$PWD/RtMidi.cpp

HEADERS += \
    $$PWD/RtMidi.hpp

linux {
    contains(DEFINES, __UNIX_JACK__) {
        DEFINES += JACK_HAS_PORT_RENAME
        LIBS += -ljack
    }
    DEFINES += __LINUX_ALSA__
    LIBS += -lasound
}
win32 {
    DEFINES += __WINDOWS_MM__
    LIBS += -lwinmm
}
macx {
    contains(DEFINES, __UNIX_JACK__) {
        DEFINES += JACK_HAS_PORT_RENAME
        LIBS += -ljack
    }
    DEFINES += __MACOSX_CORE__
    LIBS += -framework CoreMIDI -framework CoreAudio -framework CoreFoundation
}

DEFINES += __RTMIDI_DUMMY__
