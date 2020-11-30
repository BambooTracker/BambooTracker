SOURCES += \
    $$PWD/RtAudio.cpp

HEADERS += \
    $$PWD/RtAudio.hpp

win32 {
    DEFINES += __WINDOWS_DS__
    LIBS += -lole32 -lwinmm -ldsound -luser32

    greaterThan(QT_MAJOR_VERSION, 4):greaterThan(QT_MINOR_VERSION, 5) {
        DEFINES += __WINDOWS_WASAPI__
        LIBS += -lole32 -lwinmm -lksuser -lmfplat -lmfuuid -lwmcodecdspuuid
    }
}
else:macx {
    DEFINES += __MACOSX_CORE__
    LIBS += -framework CoreAudio -framework CoreFoundation -lpthread

    use_jack {
        DEFINES += __UNIX_JACK__
        LIBS += -ljack
    }
}
else:linux {
    DEFINES += __LINUX_ALSA__
    LIBS += -lasound -lpthread

    use_pulse {
        DEFINES += __LINUX_PULSE__
        LIBS += -lpulse-simple -lpulse
    }
    use_jack {
        DEFINES += __UNIX_JACK__
        LIBS += -ljack
    }
}
else:bsd {
    DEFINES += __LINUX_OSS__
    LIBS += -lpthread

    use_alsa {
        DEFINES += __LINUX_ALSA__
        LIBS += -lasound
    }
    use_pulse {
        DEFINES += __LINUX_PULSE__
        LIBS += -lpulse-simple -lpulse
    }
    use_jack {
        DEFINES += __UNIX_JACK__
        LIBS += -ljack
    }
}
