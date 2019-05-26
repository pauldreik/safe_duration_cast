# CMakelists for the root directory
#
# This cmake file is partially based on https://github.com/foonathan/type_safe,
# which is MIT licensed. I am uncertain what that means to the licensing
# of this file. To the largest extent possible in this situation, my changes are
# dual licensed under Boost 1.0 license or GPLv2 (or later, at your option).
#
# By Paul Dreik 20180923

cmake_minimum_required(VERSION 3.10)
project(libchronoconv VERSION 0.0.0 LANGUAGES CXX)

set(detail_header_files
${CMAKE_CURRENT_SOURCE_DIR}/include/detail/chronoconv_detail.hpp
${CMAKE_CURRENT_SOURCE_DIR}/include/detail/lossless_conversion.hpp
)
set(header_files
${CMAKE_CURRENT_SOURCE_DIR}/include/chronoconv.hpp
)

set(target_name chronoconv)

add_library(${target_name} INTERFACE)
#target_compile_features(${target_name}     INTERFACE cxx_std_11)
target_sources(${target_name}              INTERFACE ${detail_header_files} ${header_files})
target_include_directories(${target_name}  INTERFACE include/)
target_include_directories(${target_name}  SYSTEM INTERFACE include)
target_compile_definitions(${target_name}  INTERFACE)

#for installation
install(FILES ${header_files}        DESTINATION include)
install(FILES ${detail_header_files} DESTINATION include/detail)

if (MSVC)
    # warning level 4 and all warnings as errors
    add_compile_options(/W4 /WX)
else()
    # lots of warnings
    add_compile_options(-Wall  -Wextra -pedantic -Wno-unused-parameter -Wno-unused-variable) #)
endif()


#unit tests
option(BUILD_UNITTESTS "enables the unit tests" Off)
if(BUILD_UNITTESTS)
   enable_testing()
   add_subdirectory(tests/)
endif()

option(BUILD_EXHAUSTIVE_TESTS "enables the exhaustive tests" Off)
if(BUILD_EXHAUSTIVE_TESTS)
   enable_testing()
   add_subdirectory(exhaustivetests/)
endif()

#examples
option(BUILD_EXAMPLES "enables building the examples" Off)
if(BUILD_EXAMPLES)
   add_subdirectory(examples)
endif()


#fuzzing
option(BUILD_FUZZERS "enables building the fuzzers" Off)
if(BUILD_FUZZERS)
   add_subdirectory(fuzzing)
endif()

