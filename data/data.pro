TEMPLATE = aux

include("../qmake/variables.pri")

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
      desktopfiles.files = $$PWD/applications $$PWD/mime
      desktopfiles.path = $${DIST_INSTALL_PATH}/
      INSTALLS += desktopfiles

      iconfiles.files = $$files($$PWD/icons/*x*)
      iconfiles.path = $${DIST_INSTALL_PATH}/icons/hicolor/
      INSTALLS += iconfiles
    }
  }
}
