#!/usr/bin/env bash

set -u

CURRENT=$(dirname "${BASH_SOURCE[0]}")
source "$CURRENT/versions"

rm -rf "$BUILD_ROOT/output/*"
mkdir -p "$BUILD_ROOT/output"

"$CURRENT/webvfx-run.sh" "$CURRENT/../../demo/mlt/tests/runtests" "$CURRENT/../../demo/mlt/tests/fixtures/macos" "$BUILD_ROOT/output"
exit $?
