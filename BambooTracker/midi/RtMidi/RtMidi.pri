SOURCES += \
    $$PWD/RtMidi.cpp

HEADERS += \
    $$PWD/RtMidi.hpp

linux {
    use_jack {
        DEFINES += __UNIX_JACK__
        LIBS += -ljack
        jack_has_rename {
            DEFINES += JACK_HAS_PORT_RENAME
        }
    }
    DEFINES += __LINUX_ALSA__
    LIBS += -lasound
}
win32 {
    DEFINES += __WINDOWS_MM__
    LIBS += -lwinmm
}
macx {
    use_jack {
        DEFINES += __UNIX_JACK__
        LIBS += -ljack
        jack_has_rename {
            DEFINES += JACK_HAS_PORT_RENAME
        }
    }
    DEFINES += __MACOSX_CORE__
    LIBS += -framework CoreMIDI -framework CoreAudio -framework CoreFoundation
}

DEFINES += __RTMIDI_DUMMY__
