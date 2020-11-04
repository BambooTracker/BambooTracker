# QMake wrapper over compiled-in Qt Resource definitions

RESOURCES += \
    $$PWD/doc/doc.qrc \
    $$PWD/icon/icon.qrc

# Platform-specific app icon
win32 {
  RC_ICONS = $$PWD/icon/BambooTracker.ico
}
else:osx {
  ICON = $$PWD/icon/BambooTracker.icns
}
