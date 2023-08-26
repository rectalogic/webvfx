#!/usr/bin/env bash

set -u

CURRENT=$(dirname "${BASH_SOURCE[0]}")
source "$CURRENT/versions"

python3 -m venv --clear "${BUILD_ROOT}/build/qtvenv"
"$BUILD_ROOT/build/qtvenv/bin/pip" install --upgrade --upgrade-strategy eager aqtinstall
"$BUILD_ROOT/build/qtvenv/bin/python" -m aqt install-qt mac desktop ${QT_VER} --modules qtmultimedia qtquick3d qtshadertools qtquicktimeline qtwebchannel qtwebengine qtwebsockets qtwebview qtpositioning debug_info --archives qtbase qtdeclarative qtquick3d qtshadertools qtsvg qtwebengine qttools -O "$BUILD_ROOT/installed"
