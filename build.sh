#!/bin/sh
# License: dual license, pick your choice. Either Boost license 1.0, or GPL(v2
# or later, at your option).
# by Paul Dreik 20180919

set -e

root=$(dirname "$0")

if [ -e "$root/build" ] ;  then
   rm -rf "$root/build"
fi

mkdir -p "$root/build"

cd "$root/build"
cmake .. -GNinja

ninja

