requires(!system_rtmidi)

system_rtmidi {
  message("Vendored RtMidi disabled.")
}
else {
  include("../common/library.pri")
  TARGET = rtmidi

  pkgconfig_required = false
  pthread_required = false

  win32 {
    message("[default] Adding Windows Multimedia")
    DEFINES += __WINDOWS_MM__
    LIBS += -lwinmm
  }

  macx {
    message("[default] Adding CoreMIDI")
    DEFINES += __MACOSX_CORE__
    LIBS += -framework CoreMIDI -framework CoreAudio -framework CoreFoundation -framework CoreServices
  }

  tests_alsa = alsa
  if(libraryFeature("ALSA", linux, alsa, tests_alsa)) {
    DEFINES += __LINUX_ALSA__
    LIBS += -lasound
    pthread_required = true
  }

  tests_jack = jack_pkgconfig jack_lib jack_pkgconfig_rename jack_lib_rename
  if(libraryFeature("JACK", , jack, tests_jack)) {
    DEFINES += __UNIX_JACK__
    pthread_required = true

    config_jack_*_rename {
      DEFINES += JACK_HAS_PORT_RENAME
    }
    config_jack_pkgconfig* {
      pkgconfig_required = true
      PKGCONFIG += jack
    }
    else {
      LIBS += -ljack
    }
  }

  equals(pthread_required, "true") {
    LIBS += -lpthread
  }
  equals(pkgconfig_required, "true") {
    CONFIG += link_pkgconfig
  }

  SOURCES = $$PWD/src/RtMidi.cpp
  HEADERS = $$PWD/src/RtMidi.h
}
