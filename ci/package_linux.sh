#!/usr/bin/env bash

set -e

make -C ../BambooTracker install
export VERSION=${TRAVIS_TAG:1}
linuxdeployqt ./usr/share/applications/BambooTracker.desktop -verbose=2 -appimage
mv BambooTracker-$VERSION-*.AppImage ../BambooTracker.AppImage
rm -rf *
mv ../BambooTracker.AppImage ./

ls -ahl

exit 0
