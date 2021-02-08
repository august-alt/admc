#!/bin/bash

# -e exits on error, -u errors on undefined variables, and -o (for option) pipefail exits on command pipe failures.
set -euxo pipefail

# setup xvfb
Xvfb :1 -screen 0 1920×1080x16 &> xvfb.log  &

export DISPLAY=:1.0

# build tests
cd /app

cmake -B build -DFIND_MODULES=ON

cmake --build build/ -j4

./build/test_admc

# make package
cd /app/ && gear-rpm -ba
