#!/usr/bin/env bash

set -u

CURRENT=$(dirname "${BASH_SOURCE[0]}")
source "$CURRENT/versions"

mkdir -p "$BUILD_ROOT"

"$CURRENT/install-qt6.sh"
"$CURRENT/install-macports.sh"
"$CURRENT/install-frei0r.sh"
"$CURRENT/install-ffmpeg.sh"
"$CURRENT/install-mlt.sh"