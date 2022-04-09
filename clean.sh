#!/usr/bin/env bash

clean()
{

local readonly GIT_REPO_DIR=$(git rev-parse --show-toplevel)
local readonly BUILD_DIR="$GIT_REPO_DIR/_build"

rm -rf "$BUILD_DIR"

}

clean "$@"
unset clean
