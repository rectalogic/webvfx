#!/usr/bin/env bash

ROOT=$(dirname "${BASH_SOURCE[0]}")
docker buildx build --memory-swap -1 --tag webvfx "$ROOT"
