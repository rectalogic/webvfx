#!/usr/bin/env bash

set -u

CURRENT=$(dirname "${BASH_SOURCE[0]}")
source "$CURRENT/versions"

MLT_GIT="${BUILD_ROOT}/mlt"
"$CURRENT/git-checkout.sh" https://github.com/mltframework/mlt.git ${MLT_VER} "$MLT_GIT"
MLT_BUILD="${BUILD_ROOT}/build/mlt"
mkdir -p "$MLT_BUILD"

PATH=${INSTALL_ROOT}/bin:$PATH

cd "$MLT_BUILD"
cmake --install-prefix "$INSTALL_ROOT" -DRELOCATABLE=OFF -DMOD_GDK=OFF "$MLT_GIT"
cmake --build .
cmake --install .
