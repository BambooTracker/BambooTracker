#!/usr/bin/env bash

set -e

HELP_OUT="$(windeployqt -h)"
echo "$HELP_OUT"

DEPLOY_OPTS="-verbose=2"
PLUGIN_OPTS=("quick-import" "system-d3d-compiler" "webkit2" "opengl-sw" "virtualkeyboard", "angle")
for opt in ${PLUGIN_OPTS[@]}; do
  no_opt="--no-$opt"
  if [[ $(echo "$HELP_OUT" | grep -e "$no_opt") ]]; then
    DEPLOY_OPTS="$DEPLOY_OPTS $no_opt"
  fi
done

while [[ $# -gt 0 ]]; do
  opt="$1"
  # This check is not perfect, especially with very generic names...
  if [[ $(echo "$HELP_OUT" | grep -e "$opt") ]]; then
    DEPLOY_OPTS="$DEPLOY_OPTS --no-$opt"
  fi
  shift 1
done

cp -va ../*.md .
windeployqt BambooTracker.exe $DEPLOY_OPTS
cp -va translations/*.qm lang

# Save qico* library for working .ico support!
mv -v imageformats/*qico* ./
rm -vrf imageformats/* translations
mv -v *qico* imageformats/

exit 0

