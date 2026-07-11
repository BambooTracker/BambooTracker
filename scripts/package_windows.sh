#!/usr/bin/env bash

# Remember to port any changes in here the powershell port in .appveyor.yml!

set -e

HELP_OUT=$(windeployqt -h)
DEPLOY_OPTS="-verbose=2"
PLUGIN_OPTS=("--no-quick-import" "--no-system-d3d-compiler" "--no-webkit2" "--no-opengl-sw" "--no-virtualkeyboard", "--no-angle")
for opt in ${PLUGIN_OPTS[@]}; do
  if [[ $(echo $HELP_OUT | grep -e $opt) ]]; then
    DEPLOY_OPTS="$DEPLOY_OPTS $opt"
  fi
done

EXLIB_OPTS=("svg")
for opt in ${EXLIB_OPTS[@]}; do
  if [[ $(echo $HELP_OUT | grep -e $opt) ]]; then
    DEPLOY_OPTS="$DEPLOY_OPTS --no-$opt"
  fi
done

cp -va ../*.md .
windeployqt BambooTracker.exe $DEPLOY_OPTS
cp -va translations/*.qm lang

# Save qico* library for working .ico support!
mv -v imageformats/*qico* ./
rm -vrf imageformats/* translations
mv -v *qico* imageformats/

exit 0

