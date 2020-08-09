#!/usr/bin/env bash

set -e

cp -a "$(find ../ -name BambooTracker.app)" ../{img,demos,licenses,specs,skins,*.md,LICENSE} .
macdeployqt BambooTracker.app -verbose=2
mv ../BambooTracker/.qm/ BambooTracker.app/Contents/Resources/lang

rm -rf BambooTracker.app/Contents/Frameworks/{QtPdf.framework,QtQuick.framework,QtVirtualKeyboard.framework,QtQmlModels.framework,QtSvg.framework,QtQml.framework,QtOpenGL.framework,QtMultimediaWidgets.framework}
rm -rf BambooTracker.app/Contents/PlugIns/{platforminputcontexts,virtualkeyboard,iconengines,imageformats,audio,bearer,mediaservice}

exit 0
