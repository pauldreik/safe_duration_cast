#!/bin/sh
# License: dual license, pick your choice. Either Boost license 1.0, or GPL(v2
# or later, at your option).
# by Paul Dreik 20180919

set -e

root=$(readlink -f $(dirname "$0"))
me=$(basename $0)

# $1 - name
# $2...  passed to cmake
dobuild() {
 if [ -d $root/build-$1 ] ; then
 cd $root/build-$1
    else
 mkdir $root/build-$1
 cd $root/build-$1
 shift
 cmake -GNinja $root $@
fi
 ninja
 if [ -d tests ]; then
  ninja test
 fi
}

export CXX
export CXXFLAGS	

#make a full sanitizers debug build with the default compiler, C++17
CXXFLAGS="-fsanitize=undefined,address -g -O0 -std=c++1z -Wall -Wextra"
dobuild sanitizers_17 \
-DBUILD_EXAMPLES=On \
-DBUILD_UNITTESTS=On \
-DBUILD_EXHAUSTIVE_TESTS=On \
-DCMAKE_BUILD_TYPE=Debug 

#make sure the examples can be built with c++11
CXXFLAGS="-g -O0 -std=c++11 -Wall -Wextra"
dobuild normal_cpp11 \
-DBUILD_EXAMPLES=On \
-DBUILD_UNITTESTS=Off \
-DCMAKE_BUILD_TYPE=Debug

#make a libfuzzer build with sanitizers
CXX=clang++
CXXFLAGS="-fsanitize=fuzzer-no-link,undefined,address -g -O0 -std=c++1z"
dobuild libfuzzer_with_sanitizers \
-DBUILD_FUZZERS=On \
-DCMAKE_BUILD_TYPE=Debug \
-DFUZZ_LINKMAIN=Off \
-DFUZZ_LDFLAGS=-fsanitize=fuzzer

#make a speed measurement build
CXXFLAGS="-std=c++1z -Wall -Wextra"
dobuild speedtest \
-DBUILD_SPEED_TESTS=On \
-DBUILD_EXHAUSTIVE_TESTS=On \
-DCMAKE_BUILD_TYPE=Release

echo $me: all tests passed

