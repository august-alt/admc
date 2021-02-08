#!/bin/bash

# -e exits on error, -u errors on undefined variables, and -o (for option) pipefail exits on command pipe failures.
set -euxo pipefail

# setup xvfb
Xvfb :1 -screen 0 1920×1080x16 &> xvfb.log  &

export DISPLAY=:1.0

# build tests
cp -r /app $HOME/app

cd $HOME/app

cmake -B $HOME/build -DFIND_MODULES=ON

cmake --build $HOME/build/ -j4

$HOME/build/test_admc

# make package
cd /app/ && gear-rpm -ba
