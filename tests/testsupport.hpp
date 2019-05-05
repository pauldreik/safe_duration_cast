/*
 * By Paul Dreik 2018
 *
 * License:
 * dual license, pick your choice. Either Boost license 1.0, or GPL(v2 or later,
 * at your option).
 */
#pragma once

#if HAVE_INT128_TYPE
// will use native __int128 (works on clang and gcc, amd64)
#elif HAVE_BOOST_MULTIPRECISION
// use boost multiprecision
#include <boost/multiprecision/cpp_int.hpp>
#include <boost/multiprecision/cpp_int/limits.hpp>
#else
#error "no 128 bit integer types available"
#endif

namespace tests {
// make a 128 bit integer type
#if HAVE_INT128_TYPE
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wpedantic"
using Int128_t = __int128;
#pragma GCC diagnostic pop
#elif HAVE_BOOST_MULTIPRECISION
using Int128_t = boost::multiprecision::int128_t;
#else
#error "no 128 bit type available"
#endif
}
