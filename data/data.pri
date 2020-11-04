# Common path variables

win32|install_flat {
  DIST_INSTALL_PATH = $$PREFIX
  DATA_INSTALL_PATH = $$PREFIX
  LICENSE_INSTALL_PATH = $$PREFIX
}
else {
  DIST_INSTALL_PATH = $$PREFIX/share
  DATA_INSTALL_PATH = $${DIST_INSTALL_PATH}/BambooTracker
  LICENSE_INSTALL_PATH = $${DIST_INSTALL_PATH}/doc/BambooTracker
}

# Simple data-only targets

licenses.files = $$PWD/../licenses $$PWD/../LICENSE
licenses.path = $${LICENSE_INSTALL_PATH}
INSTALLS += licenses

!install_minimal {
  examplefiles.files = $$PWD/demos $$PWD/skins
  examplefiles.path = $${DATA_INSTALL_PATH}
  INSTALLS += examplefiles

  specfiles.files = $$PWD/specs
  specfiles.path = $${DATA_INSTALL_PATH}
  INSTALLS += specfiles

  unix {
    manpages.files = $$files($$PWD/man/*)
    manpages.path = $${DIST_INSTALL_PATH}/man/
    INSTALLS += manpages

    !macx {
      desktopfile.files = $$PWD/applications
      desktopfile.path = $${DIST_INSTALL_PATH}/
      INSTALLS += desktopfile

      iconfiles.files = $$files($$PWD/icons/*x*)
      iconfiles.path = $${DIST_INSTALL_PATH}/icons/hicolor/
      INSTALLS += iconfiles
    }
  }
}

# Resource bundle wrapper

include("resources/resources.pri")

# i18n generation & installation

QM_FILES_INSTALL_PATH = $${DATA_INSTALL_PATH}/lang

TRANSLATIONS += \
    $$PWD/lang/bamboo_tracker_fr.ts \
    $$PWD/lang/bamboo_tracker_ja.ts

equals(QT_MAJOR_VERSION, 5):lessThan(QT_MINOR_VERSION, 12) {
    message(Using a workaround for missing 'lrelease' option in Qt <5.12...)

    for(tsfile, TRANSLATIONS) {
      qmfile   = $$tsfile
      qmfile  ~= s/.ts$/.qm/
      qmfile  ~= s,/lang/,/.qm/,

      win32:$${qmfile}.commands = mkdir $$PWD/.qm;
      else:$${qmfile}.commands = test -d $$PWD/.qm/ || mkdir -p $$PWD/.qm/;
      $${qmfile}.commands += lrelease -qm $$qmfile $$tsfile
      $${qmfile}.depends = $${tsfile}

      PRE_TARGETDEPS      += $${qmfile}
      QMAKE_EXTRA_TARGETS += $${qmfile}

      translations_target = translations_$$basename(qmfile)
      $${translations_target}.depends = $$qmfile
      $${translations_target}.CONFIG = no_check_exist
      $${translations_target}.files = $$qmfile
      $${translations_target}.path = $$QM_FILES_INSTALL_PATH

      INSTALLS += $${translations_target}
    }
}
else {
    !versionAtLeast(QT_VERSION, 5.14.2) {
      message(Using a workaround for 'qm_files' target missing its install phase due to checking for the translations too early...)
      qm_files.CONFIG = no_check_exist
    }
    CONFIG += lrelease
}
