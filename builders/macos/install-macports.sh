#!/usr/bin/env bash

set -u

CURRENT=$(dirname "${BASH_SOURCE[0]}")
source "$CURRENT/versions"

MACPORTS_GIT="${BUILD_ROOT}/macports-base"
"$CURRENT/git-checkout.sh" git@github.com:macports/macports-base.git ${MACPORTS_VER} "$MACPORTS_GIT"

# Avoid any system-wide macports
export PATH=/bin:/sbin:/usr/bin:/usr/sbin
(
    cd "$MACPORTS_GIT"
    make distclean
    ./configure \
        --prefix="${INSTALL_ROOT}" \
        --with-applications-dir="$INSTALL_ROOT/Applications" \
        --without-startupitems \
    --with-no-root-privileges
    make
    make install
)
"${INSTALL_ROOT}/bin/port" -d selfupdate
"${INSTALL_ROOT}/bin/port" -N install cmake pkgconfig libsdl2
