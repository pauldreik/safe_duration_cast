/*
 * By Paul Dreik 2019
 *
 * License:
 * dual license, pick your choice. Either Boost license 1.0, or GPL(v2 or later,
 * at your option).
 */
#ifndef INCLUDE_DETAIL_SAFE_FLOAT_CONVERSION_HPP_
#define INCLUDE_DETAIL_SAFE_FLOAT_CONVERSION_HPP_

#include <cmath>
#include <limits>
#include <type_traits>

#include <detail/stdutils.hpp>

namespace safe_duration_cast {

/**
 * converts From to To if possible, otherwise ec is set.
 *
 * input                            |    output
 * ---------------------------------|---------------
 * NaN                              | NaN
 * Inf                              | Inf
 * normal, fits in output           | converted
 * normal, does not fit in output   | ec is set
 * subnormal                        | best effort
 * -Inf                             | -Inf
 */
template<typename To, typename From>
SDC_RELAXED_CONSTEXPR To
safe_float_conversion(From from, int& ec)
{
  ec = 0;
  using T = std::numeric_limits<To>;
  static_assert(std::is_floating_point<From>::value, "From must be floating");
  static_assert(std::is_floating_point<To>::value, "To must be floating");

  // catch the only happy case
  if (std::isfinite(from)) {
    if (from >= T::lowest() && from <= T::max()) {
      return static_cast<To>(from);
    }
    // not within range.
    ec = 1;
    return {};
  }

  // nan and inf will be preserved
  return static_cast<To>(from);
} // function

} // namespace
#endif /* INCLUDE_DETAIL_SAFE_FLOAT_CONVERSION_HPP_ */
