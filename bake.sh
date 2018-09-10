#!/bin/bash
set -e

if  which cmake3; then
  CMAKE_BIN="cmake3"
else
 CMAKE_BIN="cmake"
fi

echo "using $CMAKE_BIN"
$CMAKE_BIN --version

rm -rf build
mkdir build
cd build

$CMAKE_BIN  -Wno-dev  ..

make  VERBOSE=1

