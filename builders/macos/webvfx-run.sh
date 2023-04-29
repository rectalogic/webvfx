#!/usr/bin/env bash

set -u

CURRENT=$(dirname "${BASH_SOURCE[0]}")
source "$CURRENT/versions"

export FREI0R_PATH="$INSTALL_ROOT/lib/frei0r-1/webvfx"
export PATH="$INSTALL_ROOT/bin/":$PATH

"$@"
