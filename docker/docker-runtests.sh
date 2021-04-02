#!/usr/bin/env bash

mkdir -p /webvfx/build/linux
cd /webvfx/build/linux
qmake ../..
make install

cd /webvfx/demo/mlt/tests
rm -rf /webvfx/build/linux/output/*
mkdir -p /webvfx/build/linux/output
./runtests fixtures/linux /webvfx/build/linux/output
exit $?
