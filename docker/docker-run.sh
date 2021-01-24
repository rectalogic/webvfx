#!/usr/bin/env bash

ROOT=$(dirname "${BASH_SOURCE[0]}")
MOUNT=${1:-"$(dirname "$(cd "${ROOT}"; pwd)/$(basename "$1")")"}
docker buildx build --tag webvfx "$ROOT"
docker run --rm --init \
    --mount="type=bind,src=${MOUNT},dst=/webvfx,consistency=cached" webvfx
