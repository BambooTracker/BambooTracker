TEMPLATE = aux

include("../qmake/variables.pri")

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
