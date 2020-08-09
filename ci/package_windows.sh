#!/usr/bin/env bash

set -e

DEPLOY_OPTIONS="--no-quick-import --no-system-d3d-compiler --no-compiler-runtime --no-webkit2 --no-angle --no-svg"
ADDITIONAL_OPTIONS=("--no-opengl-sw" "--no-virtualkeyboard")

for opt in ${CHECK_OPTIONS[@]}; do
  if [[ $(./windeployqt.exe -h | grep -e $opt) ]]; then
    DEPLOY_OPTIONS="$DEPLOY_OPTIONS $opt"
  fi
done

cp -a "$(find ../ -name BambooTracker.exe)" ../{img,demos,licenses,specs,skins,*.md,LICENSE} .
windeployqt BambooTracker.exe -verbose=2 $DEPLOY_OPTIONS
mv translations lang
mv ../BambooTracker/.qm/*.qm lang/
rm -rf imageformats

exit 0
