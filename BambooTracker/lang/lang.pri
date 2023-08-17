# i18n generation & installation

QM_FILES_INSTALL_PATH = $${DATA_INSTALL_PATH}/lang

TRANSLATIONS += \
    $$PWD/bamboo_tracker_fr.ts \
    $$PWD/bamboo_tracker_ja.ts \
    $$PWD/bamboo_tracker_pl.ts \
    $$PWD/bamboo_tracker_es.ts \

equals(QT_MAJOR_VERSION, 5):lessThan(QT_MINOR_VERSION, 12) {
    message(Using a workaround for missing 'lrelease' option in Qt <5.12...)

    for(tsfile, TRANSLATIONS) {
      qmfile   = $$tsfile
      qmfile  ~= s/.ts$/.qm/
      qmfile  ~= s,/lang/,/.qm/,

      win32:$${qmfile}.commands = mkdir $$PWD/../.qm;
      else:$${qmfile}.commands = test -d $$PWD/../.qm/ || mkdir -p $$PWD/../.qm/;
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
