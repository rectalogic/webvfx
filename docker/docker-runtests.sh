#!/usr/bin/env bash

set -e

mkdir -p /webvfx/build/linux
cd /webvfx/build/linux
cmake --install-prefix /usr/local/Qt/${QT_VER}/gcc_64 ../..
cmake --build .
cmake --install .

cd /webvfx/demo/mlt/tests
rm -rf /webvfx/build/linux/output/*
mkdir -p /webvfx/build/linux/output
./runtests fixtures/linux /webvfx/build/linux/output
exit $?
