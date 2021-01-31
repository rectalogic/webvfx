#!/usr/bin/env bash

mkdir -p /webvfx/build/linux
cd /webvfx/build/linux
qmake ../..
make install

cd /webvfx/demo/mlt/tests
mkdir -p fixtures/output
./runtests fixtures/linux fixtures/output
exit $?
