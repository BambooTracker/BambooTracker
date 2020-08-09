#!/usr/bin/env bash

set -e

cp -a "$(find ../ -name BambooTracker.app)" ../{img,demos,licenses,specs,skins,*.md,LICENSE} .
macdeployqt BambooTracker.app -verbose=2
LANGDIR="BambooTracker.app/Contents/Resources/lang"
mv ../BambooTracker/.qm/ $LANGDIR

TRDIR="$(dirname $(which lconvert))/translations"
for lang in $(find $TRDIR/qtbase_*.qm | sed "s/^.*qtbase_\(.*\)\.qm$/\1/"); do
  lconvert -o "$LANGDIR/qt_$lang.qm" "$TRDIR/qtbase_$lang.qm" "$TRDIR/qtmultimedia_$lang.qm"
done

rm -rf BambooTracker.app/Contents/Frameworks/{QtPdf.framework,QtQuick.framework,QtVirtualKeyboard.framework,QtQmlModels.framework,QtSvg.framework,QtQml.framework,QtOpenGL.framework,QtMultimediaWidgets.framework}
rm -rf BambooTracker.app/Contents/PlugIns/{platforminputcontexts,virtualkeyboard,iconengines,imageformats,audio,bearer,mediaservice}

exit 0
