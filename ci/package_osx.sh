#!/usr/bin/env bash

set -e

cp -a "$(find ../ -name BambooTracker.app)" ../{img,demos,licenses,specs,skins,*.md,LICENSE} .
macdeployqt BambooTracker.app -verbose=2
mv ../BambooTracker/.qm/ BambooTracker.app/Contents/Resources/lang

exit 0
