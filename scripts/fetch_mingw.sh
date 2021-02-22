#!/usr/bin/env bash

set -e

if [[ "$#" -lt 3 ]]; then
  echo "Incomplete list of arguments." >&2
  echo "Need: <target-dir> <target-version> <target-bitness> [<target-cachedir>]" >&2
  exit 1
fi

MINGW_TARGETDIR="$1"
MINGW_TARGETVER="$2"
MINGW_TARGETBIT="$3"
MINGW_TARGETNEWDIR="$4"

if [[ ${MINGW_TARGETBIT} -ne 32 && ${MINGW_TARGETBIT} -ne 64 ]]; then
  echo "Invalid bitness! Must be '32' or '64', was '${MINGW_TARGETBIT}'!" >&2
  exit 1
fi

echo "Test if fetching MinGW ${MINGW_TARGETVER} is required..."

if [[
  x${MINGW_TARGETNEWDIR} == x &&
  -d ${MINGW_TARGETDIR}/bin &&
  -f ${MINGW_TARGETDIR}/bin/g++ &&
  $(${MINGW_TARGETDIR}/bin/g++ -dumpversion) == ${MINGW_TARGETVER}
  ||
  x${MINGW_TARGETNEWDIR} != x &&
  -d ${MINGW_TARGETNEWDIR}/bin &&
  -f ${MINGW_TARGETNEWDIR}/bin/g++ &&
  $(${MINGW_TARGETNEWDIR}/bin/g++ -dumpversion) == ${MINGW_TARGETVER}
]]; then
  echo "A suitable MinGW installation already seems to exist at ${MINGW_TARGETNEWDIR:-$MINGW_TARGETDIR}!"
  echo "If this is incorrect, please invalidate the cache and restart the build."
  exit 0
else
  echo "Fetching ${MINGW_TARGETBIT}-bit MinGW ${MINGW_TARGETVER}."
  MINGW_TARGETARGS=
  if [[ ${MINGW_TARGETBIT} -eq 32 ]]; then
    MINGW_TARGETARGS='-x86 -params "/exception:dwarf"'
  fi
  choco install -q mingw --version=${MINGW_TARGETVER} ${MINGW_TARGETARGS} --force
  if [[ x${MINGW_TARGETNEWDIR} != x ]]; then
    echo "Cache-helper enabled."
    echo "Copying ${MINGW_TARGETBIT}-bit MinGW ${MINGW_TARGETVER} from"
    echo "'${MINGW_TARGETDIR}' to"
    echo "'${MINGW_TARGETNEWDIR}'"
    echo "and reassigning it to $(whoami)."
    mkdir -vp $(dirname ${MINGW_TARGETNEWDIR})
    cp -va ${MINGW_TARGETDIR} ${MINGW_TARGETNEWDIR}
    chown -R $(whoami) ${MINGW_TARGETNEWDIR}
    chmod -R 755 ${MINGW_TARGETNEWDIR}
  fi
  echo "${MINGW_TARGETBIT}-bit MinGW ${MINGW_TARGETVER} is fetched."
  exit 0
fi

