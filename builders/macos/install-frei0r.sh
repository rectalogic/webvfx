#!/usr/bin/env bash

set -u

CURRENT=$(dirname "${BASH_SOURCE[0]}")
source "$CURRENT/versions"

FREI0R_GIT="${BUILD_ROOT}/frei0r"
"$CURRENT/git-checkout.sh" https://github.com/dyne/frei0r.git ${FREI0R_VER} "$FREI0R_GIT"
FREI0R_BUILD="${BUILD_ROOT}/build/frei0r"
mkdir -p "$FREI0R_BUILD"

PATH=${INSTALL_ROOT}/bin:$PATH

cd "$FREI0R_BUILD"
cmake --install-prefix "$INSTALL_ROOT" "$FREI0R_GIT"
cmake --build .
cmake --install .

