#!/usr/bin/env bash

set -e

sudo mkdir -p /webvfx/build/linux && sudo chown webvfx /webvfx/build/linux
cd /webvfx/build/linux
cmake --install-prefix /usr/local/Qt/${QT_VER}/gcc_64 ../..
cmake --build .
sudo cmake --install .

rm -rf /webvfx/build/linux/output/*
mkdir -p /webvfx/build/linux/output
export FREI0R_PATH=/usr/lib/frei0r-1/webvfx
exec "$@"
