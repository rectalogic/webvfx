#!/usr/bin/env bash

set -u

CURRENT=$(dirname "${BASH_SOURCE[0]}")
source "$CURRENT/versions"

WEBVFX_ROOT="$(cd "$(dirname "${CURRENT}")/../"; pwd)"

WEBVFX_BUILD="${BUILD_ROOT}/build/webvfx"
mkdir -p "$WEBVFX_BUILD"

PATH=$INSTALL_ROOT/bin:$PATH

cd "$WEBVFX_BUILD"
cmake --install-prefix "$INSTALL_ROOT" "$WEBVFX_ROOT"
cmake --build .
cmake --install .
