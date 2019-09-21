SOURCES += \
    $$PWD/RtAudio.cpp

HEADERS += \
    $$PWD/RtAudio.h

linux {
    DEFINES += __LINUX_ALSA__
    LIBS += -lasound -lpthread
    DEFINES += __LINUX_PULSE__
    LIBS += -lpulse-simple -lpulse
    DEFINES += __UNIX_JACK__
    LIBS += -ljack
}
win32 {
    DEFINES += __WINDOWS_DS__
    LIBS += -lole32 -lwinmm -ldsound
    greaterThan(QT_MAJOR_VERSION, 4) {
        greaterThan(QT_MINOR_VERSION, 5) {
            DEFINES += __WINDOWS_WASAPI__
            LIBS += -lksuser -lmfplat -lmfuuid -lwmcodecdspuuid
        }
    }
}
macx {
    DEFINES += __MACOSX_CORE__
    LIBS += -framework CoreAudio -framework CoreFoundation -lpthread
}
