/*
 * By Paul Dreik 2019
 *
 * License:
 * dual license, pick your choice. Either Boost license 1.0, or GPL(v2 or later,
 * at your option).
 *
 * This file is here to make it easier to support multiple C++ versions,
 * C++17 is not everywhere yet.
 */

#pragma once

// see
// https://en.cppreference.com/w/User:D41D8CD98F/feature_testing_macros#C.2B.2B17
#if __cpp_if_constexpr >= 201606
#define SDC_CONSTEXPR_IF constexpr
#else
#define SDC_CONSTEXPR_IF
#endif

#if __cpp_constexpr >= 201304
#define SDC_RELAXED_CONSTEXPR constexpr
#else
#define SDC_RELAXED_CONSTEXPR
#endif
