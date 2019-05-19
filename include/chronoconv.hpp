#include <stdexcept>

#include <detail/chronoconv_detail.hpp>

namespace safe_duration_cast {
/**
 * converts From to To (potentially lossy, just like std::chrono::duration_cast,
 * but avoids internal overflow.
 *
 * if the conversion is from an integral type to another - all types of error
 * are caught, that is, either the correct result is obtained, or the error flag
 * is set. undefined behaviour from
 *
 * for conversions between floating point values, the situation is more tricky.
 *
 * value       |   result
 * ---------------------------
 * NaN         |   NaN
 * +Inf        |   +Inf
 * "normal"    |   the correct result, or ec is set.
 * subnormal   |   best effort
 * -Inf        |   -Inf
 *
 *
 * conversions between integral and floating point is not yet supported and wont
 * compile.
 *
 * types not recognized as either integral or floatinpoint, will be directed to
 * std::chrono::duration_cast
 */
template<typename To, typename From>
constexpr To
safe_duration_cast(From from, int& ec)
{
  ec = 0;
  static_assert(detail::is_duration(From{}), "From is not a duration");
  static_assert(detail::is_duration(To{}), "To is not a duration");

  constexpr bool From_is_integral = detail::is_integral_duration(From{});
  constexpr bool To_is_integral = detail::is_integral_duration(To{});
  constexpr bool From_is_floating = detail::is_floating_duration(From{});
  constexpr bool To_is_floating = detail::is_floating_duration(To{});
  if constexpr (From_is_integral && To_is_integral) {
    const auto to = detail::duration_cast_int2int<To>(from, ec);
    return to;
  }

  if constexpr (From_is_floating && To_is_floating) {
    assert(std::fetestexcept(FE_INVALID) == 0);
    const auto to = detail::duration_cast_float2float<To>(from, ec);
    assert(std::fetestexcept(FE_INVALID) == 0);
    return to;
  }

  static_assert(!(From_is_integral && To_is_floating),
                "integral->float not supported yet");
  static_assert(!(From_is_floating && To_is_integral),
                "float->integral not supported yet");

  // fallback to std for cases not caught above
  return std::chrono::duration_cast<To>(from);
}

// throwing version
template<typename To, typename From>
To
safe_duration_cast(From from)
{
  int ec = 0;
  auto ret = safe_duration_cast<To, From>(from, ec);
  if (ec) {
    throw std::runtime_error("failed conversion");
  }
  return ret;
} // func
} // namespace safe_duration_cast
