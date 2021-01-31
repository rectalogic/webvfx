#!/usr/bin/env bash

MOUNT="$(dirname "$(cd "$(dirname "${BASH_SOURCE[0]}")"; pwd)")"
docker run --rm --init \
    --mount="type=bind,src=${MOUNT},dst=/webvfx,consistency=cached" webvfx "$@"
