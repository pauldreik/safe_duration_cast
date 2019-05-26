#!/bin/sh
# License: dual license, pick your choice. Either Boost license 1.0, or GPL(v2
# or later, at your option).
# by Paul Dreik 20180919

set -e

root=$(readlink -f $(dirname "$0"))
me=$(basename $0)

stds="11 1y 1z 2a"

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

#make a sanitizers debug build with the default compiler, C++17
CXXFLAGS="-fsanitize=undefined,address -g -O0 -std=c++1z -Wall -Wextra"
dobuild sanitizers_17 \
-DBUILD_EXAMPLES=On \
-DBUILD_FUZZERS=Off \
-DBUILD_UNITTESTS=On \
-DCMAKE_BUILD_TYPE=Debug \
-DFUZZ_LINKMAIN=On

#make sure the examples can be built with c++11
CXXFLAGS="-g -O0 -std=c++11 -Wall -Wextra"
dobuild normal_cpp11 \
-DBUILD_EXAMPLES=On \
-DBUILD_FUZZERS=Off \
-DBUILD_UNITTESTS=Off \
-DCMAKE_BUILD_TYPE=Debug \
-DFUZZ_LINKMAIN=On

#make a libfuzzer build with sanitizers
CXX=clang++
CXXFLAGS="-fsanitize=fuzzer-no-link,undefined,address -g -O0 -std=c++1z"
dobuild libfuzzer_with_sanitizers \
-DBUILD_EXAMPLES=Off \
-DBUILD_FUZZERS=On \
-DBUILD_UNITTESTS=Off \
-DCMAKE_BUILD_TYPE=Debug \
-DFUZZ_LINKMAIN=Off \
-DFUZZ_LDFLAGS=-fsanitize=fuzzer

echo $me: all tests passed
exit 0

mkdir "$root/build-sanitizers"
cd "$root/build-sanitizers"
CXX=clang++ CXXFLAGS="-fsanitize=undefined,address -g -O0" \
cmake $root -DBUILD_EXAMPLES=On -DBUILD_FUZZERS=Off -DBUILD_UNITTESTS=On \
-DCMAKE_BUILD_TYPE=Debug -DFUZZ_LINKMAIN=On -GNinja
ninja

#make a fuzzer build
mkdir "$root/build-libfuzzer"
cd "$root/build-libfuzzer"
CXX=clang++ CXXFLAGS="-fsanitize=fuzzer-no-link,undefined,address" \
cmake $root -DBUILD_EXAMPLES=Off -DBUILD_FUZZERS=On -DBUILD_UNITTESTS=Off \
-DCMAKE_BUILD_TYPE=Debug -DFUZZ_LINKMAIN=Off -GNinja -DFUZZ_LDFLAGS=-fsanitize=fuzzer
ninja

