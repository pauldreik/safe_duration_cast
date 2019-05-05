# Copyright (c) 2019, Paul Dreik

# settings this links in a main. useful for reproducing,
# kcov, gdb, afl, valgrind.
# (note that libFuzzer can also reproduce, just pass it the files)
option(FUZZ_LINKMAIN "enables the reproduce mode, instead of libFuzzer" On)

#for oss-fuzz - insert $LIB_FUZZING_ENGINE into the link flags, but only for
#the fuzz targets, otherwise the cmake configuration step fails.
set(FUZZ_LDFLAGS "" CACHE STRING "LDFLAGS for the fuzz targets")

#find all fuzzers.
set(SOURCES
chrono.cpp
)

if(FUZZ_LINKMAIN)
  add_definitions(-DIMPLEMENT_MAIN=1)
endif()

macro(implement_fuzzer sourcefile)
  get_filename_component(basename ${sourcefile} NAME_WE)
  set(name fuzzer_${basename})
  add_executable(${name} ${sourcefile})
  target_link_libraries(${name}  PRIVATE chronoconv)
if(FUZZ_LDFLAGS)
  target_link_libraries(${name} PRIVATE ${FUZZ_LDFLAGS})
endif()
  target_compile_features(${name} PRIVATE cxx_generic_lambdas)
endmacro()

foreach(X IN ITEMS ${SOURCES})
    implement_fuzzer(${X})
endforeach()
