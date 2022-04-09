#!/usr/bin/env bash

clean()
{

local readonly GIT_REPO_DIR=$(git rev-parse --show-toplevel)
local readonly BUILD_DIR="./_build"
local readonly CACHE_DIR="./.cache"

rm -rf "$BUILD_DIR"
rm -rf "$CACHE_DIR"

}

clean "$@"
unset clean
