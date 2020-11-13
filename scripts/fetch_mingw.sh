#!/usr/bin/env bash

set -e

if [[ "$#" -lt 2 ]]; then
  echo "Incomplete list of arguments." >&2
  echo "Need: <target-dir> <target-version>" >&2
  exit 1
fi

MINGW_TARGETDIR="$1"
MINGW_TARGETVER="$2"
MINGW_TARGETNEWDIR="$3"

echo "Test if fetching x86 MinGW ${MINGW_TARGETVER} is required..."

if [[
  -d ${MINGW_TARGETDIR}/bin &&
  -f ${MINGW_TARGETDIR}/bin/g++ &&
  $(${MINGW_TARGETDIR}/bin/g++ -dumpversion) == ${MINGW_TARGETVER}
  ||
  x${MINGW_TARGETNEWDIR} != x &&
  -d ${MINGW_TARGETNEWDIR}/bin &&
  -f ${MINGW_TARGETNEWDIR}/bin/g++ &&
  $(${MINGW_TARGETNEWDIR}/bin/g++ -dumpversion) == ${MINGW_TARGETVER}
]]; then
  echo "x86 MinGW ${MINGW_TARGETVER} seems cached."
  echo "If cache is incorrect, please invalidate the cache and restart the build."
  exit 0
else
  echo "Fetching x86 MinGW ${MINGW_TARGETVER}."
  choco install -q mingw --version=${MINGW_TARGETVER} -x86 -params "/exception:dwarf" --force
  if [ x${MINGW_TARGETNEWDIR} != x ]; then
    echo "Override directory specified, copying x86 MinGW ${MINGW_TARGETVER} from"
    echo "'${MINGW_TARGETDIR}' to"
    echo "'${MINGW_TARGETNEWDIR}'"
    echo "and reassigning it to $(whoami)."
    mkdir -vp $(dirname ${MINGW_TARGETNEWDIR})
    cp -va ${MINGW_TARGETDIR} ${MINGW_TARGETNEWDIR}
    chown -R $(whoami) ${MINGW_TARGETNEWDIR}
    chmod -R 755 ${MINGW_TARGETNEWDIR}
  fi
  echo "x86 MinGW ${MINGW_TARGETVER} is fetched."
  exit 0
fi

