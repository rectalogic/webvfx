#!/usr/bin/env bash

ROOT=$(dirname "${BASH_SOURCE[0]}")
MOUNT="$(dirname "$(cd "${ROOT}"; pwd)")"
docker buildx build --tag webvfx "$ROOT"
docker run --rm --init \
    --mount="type=bind,src=${MOUNT},dst=/webvfx,consistency=cached" webvfx "$@"
