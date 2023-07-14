#!/usr/bin/env bash

REPO=${1:?repo}
VERSION=${2:?version}
DEST=${3:?dest}

if [ ! -d "$DEST" ]; then
    git clone "$REPO" "$DEST"
fi

if [ "$(git -C "$DEST" describe)" != "$VERSION" ]; then
    git -C "$DEST" fetch
    git -C "$DEST" checkout "$VERSION"
    git -C "$DEST" submodule update --init
fi
