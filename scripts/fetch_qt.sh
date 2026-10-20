#!/usr/bin/env bash

set -e

if [[ "$#" -lt 5 ]]; then
  echo "Incomplete list of arguments." >&2
  echo "Need: <target-dir> <target-version> <target-os> <target-toolchain> <target-component> [<target-component>...]" >&2
  exit 1
fi

QT_TARGETDIR="$1"
QT_TARGETVER="$2"
QT_TARGETOS="$3"
QT_TARGETTCH="$4"

shift 4

echo "Test if fetching Qt freshly is required."

if [[ -d "$QT_TARGETDIR"/"$QT_TARGETVER" ]]; then
  echo "Qt seems cached."
  echo "If cache is incorrect, please delete the cache and restart the build."
  exit 0
else
  curl https://code.qt.io/cgit/qbs/qbs.git/plain/scripts/install-qt.sh > install-qt.sh
  bash ./install-qt.sh -d "$QT_TARGETDIR" --host "$QT_TARGETOS" --version "$QT_TARGETVER" --toolchain "$QT_TARGETTCH" "$@"
  echo "Qt fetched."
  if [[ "$(echo "$QT_TARGETCMP" | grep essentials 2>&1 >/dev/null; echo $?)" -eq 0 ]]; then
    echo "Older Qt version detected, manually patching setup."
    export QTPATH="$(echo "$QT_TARGETDIR"/"$QT_TARGETVER"/*)"
    cat >"$QTPATH"/bin/qt.conf <<EOF
[Paths]
Prefix = ..
EOF
    sed -i -e 's/Enterprise/OpenSource/g' -e 's/licheck.*//g' "$QTPATH"/mkspecs/qconfig.pri
    echo "Qt patched."
  fi
  exit 0
fi
