#!/usr/bin/env bash

set -e

if [[ "$#" -lt 2 ]]; then
  echo "Incomplete list of arguments." >&2
  echo "Need: <target-dir> <target-version>" >&2
  exit 1
fi

MINGW_TARGETDIR="$1"
MINGW_TARGETVER="$2"

echo "Test if fetching MinGW freshly is required."

if [[ -d "$MINGW_TARGETDIR"/bin && $("$MINGW_TARGETDIR"/bin/g++ -dumpversion) == "$MINGW_TARGETVER" ]]; then
  echo "MinGW seems cached."
  echo "If cache is incorrect, please delete the cache and restart the build."
  exit 0
else
  echo "Uninstalling currently installed MinGW."
  choco uninstall -y mingw
  echo "Fetching requested x86 version of MinGW."
  choco install -q mingw --version="$MINGW_TARGETVER" -x86 -params "/exception:dwarf"
  echo "MinGW fetched."
  exit 0
fi

