SOURCES += \
    $$PWD/RtAudio.cpp

HEADERS += \
    $$PWD/RtAudio.h

linux {
    defined(__LINUX_PULSE__) {
        # DEFINES += __LINUX_PULSE__
        LIBS += -lpthread -lpulse-simple -lpulse
    }
    else:defined(__UNIX_JACK__) {
        # DEFINES += __UNIX_JACK__
        LIBS += -ljack -lpthread
    }
    else {
        DEFINES += __LINUX_ALSA__
        LIBS += -lasound -lpthread
    }
}
win32 {
    greaterThan(QT_MAJOR_VERSION, 4):greaterThan(QT_MINOR_VERSION, 5) {
        DEFINES += __WINDOWS_WASAPI__
        LIBS += -lole32 -lwinmm -lksuser -lmfplat -lmfuuid -lwmcodecdspuuid
    }
    else {
        # For Windows XP
        DEFINES += __WINDOWS_DS__
        LIBS += -lole32 -lwinmm -ldsound
    }
}
macx {
    defined(__UNIX_JACK__) {
        # DEFINES += __UNIX_JACK__
        LIBS += -ljack -lpthread
    }
    else {
        DEFINES += __MACOSX_CORE__
        LIBS += -framework CoreAudio -framework CoreFoundation -lpthread
    }
}
