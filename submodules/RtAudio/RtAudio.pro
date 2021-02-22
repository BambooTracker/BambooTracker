requires(!system_rtaudio)

system_rtaudio {
  message("Vendored RtAudio disabled.")
}
else {
  include("../common/library.pri")

  CONFIG(debug, debug|release):TARGET = rtaudiod
  else:CONFIG(release, debug|release):TARGET = rtaudio

  pkgconfig_required = false
  pthread_required = false

  win32 {
    message("[default] Adding DirectSound")
    DEFINES += __WINDOWS_DS__
    LIBS += -lole32 -lwinmm -ldsound -luser32

    if(equals(QT_MAJOR_VERSION, 5):greaterThan(QT_MINOR_VERSION, 5))|greaterThan(QT_MAJOR_VERSION, 5) {
      message("[default] Adding WASAPI")
      DEFINES += __WINDOWS_WASAPI__
      LIBS += -lksuser -lmfplat -lmfuuid -lwmcodecdspuuid
    }
  }

  macx {
    message("[default] Adding CoreAudio")
    DEFINES += __MACOSX_CORE__
    LIBS += -framework CoreAudio -framework CoreFoundation
  }

  tests_alsa = alsa
  if(libraryFeature("ALSA", linux, alsa, tests_alsa)) {
    DEFINES += __LINUX_ALSA__
    LIBS += -lasound
    pthread_required = true
  }

  tests_oss = oss oss_sys oss_ossaudio oss_sys_ossaudio
  if(libraryFeature("OSS4", bsd, oss, tests_oss)) {
    DEFINES += __LINUX_OSS__
    pthread_required = true

    config_oss_*ossaudio {
      LIBS += -lossaudio
    }
  }

  tests_pulse = pulse_pkgconfig pulse_lib
  if(libraryFeature("PulseAudio", , pulse, tests_pulse)) {
    DEFINES += __LINUX_PULSE__
    pthread_required = true

    config_pulse_pkgconfig {
      pkgconfig_required = true
      PKGCONFIG += libpulse-simple
    }
    else {
      LIBS += -lpulse -lpulse-simple
    }
  }

  tests_jack = jack_pkgconfig jack_lib jack_pkgconfig_rename jack_lib_rename
  if(libraryFeature("JACK", , jack, tests_jack)) {
    DEFINES += __UNIX_JACK__
    pthread_required = true

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

  SOURCES = $$PWD/src/Rtaudio.cpp
  HEADERS = $$PWD/src/RtAudio.h
}
