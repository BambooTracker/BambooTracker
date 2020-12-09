requires(!system_rtaudio)
# system_rtaudio {
#   !packagesExist(rtaudio) {
#     error("System-installed RtAudio could not be found! (system_rtaudio is set)")
#   }
# }
# else {

include("../common/library.pri")
TARGET = rtaudio

qtCompileTest(alsa)
qtCompileTest(oss)
qtCompileTest(oss_ossaudio)
qtCompileTest(oss_sys)
qtCompileTest(oss_sys_ossaudio)
qtCompileTest(pulse_pkgconfig)
qtCompileTest(pulse_lib)
qtCompileTest(jack_pkgconfig)
qtCompileTest(jack_lib)
qtCompileTest(jack_pkgconfig_rename)
qtCompileTest(jack_lib_rename)

pkgconfig_required = false
pthread_required = false

win32 {
  message("[default] Adding DirectSound")
  DEFINES += __WINDOWS_DS__
  LIBS += -lole32 -lwinmm -ldsound -luser32

  greaterThan(QT_MAJOR_VERSION, 4):greaterThan(QT_MINOR_VERSION, 5) {
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

linux|if(!linux:if(use_alsa|config_alsa)) {
  linux: type = "default"
  else:use_alsa: type = "enabled"
  else: type = "detected"

  message("["$${type}"]" "Adding ALSA")
  DEFINES += __LINUX_ALSA__
  LIBS += -lasound
  pthread_required = true
}

bsd|if(!bsd:if(use_oss|config_oss*)) {
  bsd: type = "default"
  else:use_oss: type = "enabled"
  else: type = "detected"

  message("["$${type}"]" "Adding OSS4")
  DEFINES += __LINUX_OSS__
  pthread_required = true

  config_oss_*ossaudio {
    LIBS += -lossaudio
  }
}

use_pulse|config_pulse_* {
  use_pulse: type = "enabled"
  else: type = "detected"

  message("["$${type}"]" "Adding PulseAudio")
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

use_jack|config_jack_* {
  use_jack: type = "enabled"
  else: type = "detected"

  message("["$${type}"]" "Adding JACK")
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
