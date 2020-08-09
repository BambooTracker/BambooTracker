#!/usr/bin/env bash

set -e

cp -a "$(find ../ -name BambooTracker.app)" ../{img,demos,licenses,specs,skins,*.md,LICENSE} .
macdeployqt BambooTracker.app -verbose=2
LANGDIR="BambooTracker.app/Contents/Resources/lang"
mv ../BambooTracker/.qm/ $LANGDIR

find "$(dirname $(dirname $(which qmake)))/translations" -name "qtbase*" | sed -e "s%^\(.*qtbase_\(.*\)\)$%cp \1 $LANGDIR/qt_\2%g" | bash

rm -rf BambooTracker.app/Contents/Frameworks/{QtPdf.framework,QtQuick.framework,QtVirtualKeyboard.framework,QtQmlModels.framework,QtSvg.framework,QtQml.framework,QtOpenGL.framework,QtMultimediaWidgets.framework}
rm -rf BambooTracker.app/Contents/PlugIns/{platforminputcontexts,virtualkeyboard,iconengines,imageformats,audio,bearer,mediaservice}

exit 0
