#!/usr/bin/env bash

set -e

cp -va ../*.md .
macdeployqt BambooTracker.app -verbose=2

LANGDIR="BambooTracker.app/Contents/Resources/lang"
mv lang $LANGDIR
find "$(dirname $(dirname $(realpath $(which qmake))))/translations" -type f -name 'qtbase_*.qm' -print0 \
| while IFS= read -r -d '' qtbasetranslation; do
  cp -v "$qtbasetranslation" "$(echo $qtbasetranslation | sed -e 's,.*qtbase_,'"$LANGDIR"'/qt_,g')"
done

# Save qico* library for working .ico support!
mv -v BambooTracker.app/Contents/PlugIns/imageformats/*qico* ./
rm -vrf BambooTracker.app/Contents/Frameworks/Qt{Pdf,Quick,VirtualKeyboard,QmlModels,Svg,Qml,OpenGL,MultimediaWidgets}.framework
rm -vrf BambooTracker.app/Contents/PlugIns/{platforminputcontexts,virtualkeyboard,iconengines,imageformats/*,audio,bearer,mediaservice}
mv -v *qico* BambooTracker.app/Contents/PlugIns/imageformats/

exit 0

