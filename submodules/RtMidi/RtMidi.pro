requires(!system_rtmidi)
# system_rtmidi {
#   !packagesExist(rtmidi) {
#     error("System-installed RtMidi could not be found! (system_rtmidi is set)")
#   }
# }
# else {

include("../common/library.pri")
TARGET = rtmidi

qtCompileTest(alsa)
qtCompileTest(jack_pkgconfig)
qtCompileTest(jack_lib)
qtCompileTest(jack_pkgconfig_rename)
qtCompileTest(jack_lib_rename)

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

linux|if(!linux:if(use_alsa|config_alsa)) {
  linux: type = "default"
  else:use_alsa: type = "enabled"
  else: type = "detected"

  message("["$${type}"]" "Adding ALSA Sequencer")
  DEFINES += __LINUX_ALSA__
  LIBS += -lasound
  pthread_required = true
}

use_jack|config_jack_* {
  use_jack: type = "enabled"
  else: type = "detected"

  message("["$${type}"]" "Adding JACK")
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
