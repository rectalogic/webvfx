#!/usr/bin/env bash

set -u

CURRENT=$(dirname "${BASH_SOURCE[0]}")
source "$CURRENT/versions"

FFMPEG_GIT="${BUILD_ROOT}/ffmpeg"
"$CURRENT/git-checkout.sh" https://git.ffmpeg.org/ffmpeg.git ${FFMPEG_VER} "$FFMPEG_GIT"
FFMPEG_BUILD="${BUILD_ROOT}/build/ffmpeg"
mkdir -p "$FFMPEG_BUILD"

PATH=${INSTALL_ROOT}/bin:$PATH
cd "$FFMPEG_BUILD"
"$FFMPEG_GIT/configure" --prefix="$INSTALL_ROOT" --extra-cflags="-I $INSTALL_ROOT/include" --enable-frei0r --enable-rpath --enable-shared --enable-gpl --disable-indevs --enable-indev=lavfi --disable-outdevs --enable-outdev=sdl2
make
make install
