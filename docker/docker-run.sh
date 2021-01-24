#!/usr/bin/env bash

MOUNT=${1:-"$(dirname "$(cd "$(dirname "${BASH_SOURCE[0]}")"; pwd)/$(basename "$1")")"}
docker buildx build --tag webvfx .
docker run --rm --init \
    --mount="type=bind,src=${MOUNT},dst=/webvfx,consistency=cached" webvfx
