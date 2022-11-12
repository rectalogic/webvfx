#!/usr/bin/env bash

ROOT=$(dirname "${BASH_SOURCE[0]}")
docker buildx build --platform linux/amd64 --memory-swap -1 --load --tag webvfx "$ROOT"
