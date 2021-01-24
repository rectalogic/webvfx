#!/usr/bin/env bash

mkdir -p /webvfx/build
cd /webvfx/build
qmake ..
make install

cd /webvfx/demo/mlt/tests
mkdir -p fixtures/output
./runtests fixtures/linux fixtures/output
exit $?
