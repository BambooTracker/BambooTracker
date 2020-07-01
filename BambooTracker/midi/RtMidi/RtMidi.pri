SOURCES += \
    $$PWD/RtMidi.cpp

HEADERS += \
    $$PWD/RtMidi.hpp

win32 {
    DEFINES += __WINDOWS_MM__
    LIBS += -lwinmm
}
else:macx {
    DEFINES += __MACOSX_CORE__
    LIBS += -framework CoreMIDI -framework CoreAudio -framework CoreFoundation

    use_jack {
        DEFINES += __UNIX_JACK__
        LIBS += -ljack
        jack_has_rename {
            DEFINES += JACK_HAS_PORT_RENAME
        }
    }
}
else:linux {
    DEFINES += __LINUX_ALSA__
    LIBS += -lasound

    use_jack {
        DEFINES += __UNIX_JACK__
        LIBS += -ljack
        jack_has_rename {
            DEFINES += JACK_HAS_PORT_RENAME
        }
    }
}
else:bsd {
    # OSS does not offer MIDI functionalities, only fallback to dummy interface
    use_alsa {
        DEFINES += __LINUX_ALSA__
        LIBS += -lasound
    }
    use_jack {
        DEFINES += __UNIX_JACK__
        LIBS += -ljack
        jack_has_rename {
            DEFINES += JACK_HAS_PORT_RENAME
        }
    }
}
