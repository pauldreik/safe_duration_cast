#!/bin/sh
# License: dual license, pick your choice. Either Boost license 1.0, or GPL(v2
# or later, at your option).
# by Paul Dreik 20180919

set -e

root=$(readlink -f $(dirname "$0"))

if [ ! -e "$root/build" ] ;  then
cd "$root/build"
cmake .. -GNinja

ninja
fi


#make a fuzzer build
mkdir "$root/build-libfuzzer"
cd "$root/build-libfuzzer"
CXX=clang++ CXXFLAGS="-fsanitize=fuzzer-no-link,undefined,address" \
cmake $root -DBUILD_EXAMPLES=Off -DBUILD_FUZZERS=On -DBUILD_UNITTESTS=Off \
-DCMAKE_BUILD_TYPE=Debug -DFUZZ_LINKMAIN=Off -GNinja -DFUZZ_LDFLAGS=-fsanitize=fuzzer
ninja

