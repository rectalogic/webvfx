#!/usr/bin/env bash

set -u

CURRENT=$(dirname "${BASH_SOURCE[0]}")
source "$CURRENT/versions"

mkdir -p "$BUILD_ROOT"

"$CURRENT/install-qt6.sh" 2>&1 | tee "$BUILD_ROOT/qt.log"
"$CURRENT/install-macports.sh" 2>&1 | tee "$BUILD_ROOT/macports.log"
"$CURRENT/install-frei0r.sh" 2>&1 | tee "$BUILD_ROOT/frei0r.log"
"$CURRENT/install-ffmpeg.sh" 2>&1 | tee "$BUILD_ROOT/ffmpeg.log"
"$CURRENT/install-mlt.sh" 2>&1 | tee "$BUILD_ROOT/mlt.log"
