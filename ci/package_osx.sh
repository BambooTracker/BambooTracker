#!/usr/bin/env bash

set -e

cp -va ../{img,*.md} .
macdeployqt BambooTracker.app -verbose=2

LANGDIR="BambooTracker.app/Contents/Resources/lang"
mv lang $LANGDIR
ls -ahl "$LANGDIR"
find "$(dirname $(dirname $(realpath $(which qmake))))/translations" -type f -name 'qtbase_*.qm' -print0 \
| while IFS= read -r -d '' qtbasetranslation; do
  cp -v "$qtbasetranslation" "$(echo $qtbasetranslation | sed -e 's,.*qtbase_,'"$LANGDIR"'/qt_,g')"
done

rm -vrf BambooTracker.app/Contents/Frameworks/Qt{Pdf,Quick,VirtualKeyboard,QmlModels,Svg,Qml,OpenGL,MultimediaWidgets}.framework
rm -vrf BambooTracker.app/Contents/PlugIns/{platforminputcontexts,virtualkeyboard,iconengines,imageformats,audio,bearer,mediaservice}

exit 0
