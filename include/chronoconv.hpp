/*
 * By Paul Dreik 2019
 *
 * License:
 * dual license, pick your choice. Either Boost license 1.0, or GPL(v2 or later,
 * at your option).
 */

#include <detail/chronoconv_detail.hpp>

// support compiling with exceptions disabled
// this works in gcc>=5 and clang>=3.6 (have not tested visual studio)
#if __cpp_exceptions >= 199711
#define SAFE_CHRONO_CONV_HAVE_EXCEPTIONS 1
#include <stdexcept>
#endif

namespace safe_duration_cast {
/**
 * A safe version of std::chrono_duration_cast, reporting an error instead
 * of invoking undefined behaviour through internal overflows and casts.
 *
 * if the conversion is from an integral type to another - all types of error
 * are caught, that is, either the correct result is obtained, or the error flag
 * is set. undefined behaviour from signed integral overflow is avoided.
 *
 * for conversions between floating point values, the conversion is as follows:
 *
 * input       |   result
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
 * types not recognized as either integral or floating point (asking
 * std::numeric_limits), will result in a compilation failure
 */
template<typename To, typename FromRep, typename FromPeriod>
constexpr To
safe_duration_cast(std::chrono::duration<FromRep, FromPeriod> from, int& ec)
{
  using From = std::chrono::duration<FromRep, FromPeriod>;
  ec = 0;
  static_assert(detail::is_duration(From{}), "From is not a duration");
  static_assert(detail::is_duration(To{}), "To is not a duration");

  constexpr bool From_is_integral = detail::is_integral_duration(From{});
  constexpr bool To_is_integral = detail::is_integral_duration(To{});
  constexpr bool From_is_floating = detail::is_floating_duration(From{});
  constexpr bool To_is_floating = detail::is_floating_duration(To{});

  static_assert(!(From_is_integral && To_is_floating),
                "integral->float not supported yet");
  static_assert(!(From_is_floating && To_is_integral),
                "float->integral not supported yet");

  static_assert(From_is_floating || From_is_integral || To_is_floating ||
                  To_is_integral,
                "conversion between non-arithmetic representations (see "
                "std::is_arithmetic<>) is not supported");

  using FromTag =
    typename detail::conditional3<From_is_integral,
                                  detail::tags::FromIsInt,
                                  From_is_floating,
                                  detail::tags::FromIsFloat,
                                  detail::tags::NotArithmetic>::type;
  using ToTag =
    typename detail::conditional3<To_is_integral,
                                  detail::tags::ToIsInt,
                                  To_is_floating,
                                  detail::tags::ToIsFloat,
                                  detail::tags::NotArithmetic>::type;
  return safe_duration_cast<To>(from, ec, FromTag{}, ToTag{});
}

#if SAFE_CHRONO_CONV_HAVE_EXCEPTIONS
// throwing version
template<typename To, typename FromRep, typename FromPeriod>
constexpr To
safe_duration_cast(std::chrono::duration<FromRep, FromPeriod> from)
{
  int ec = 0;
  auto ret = safe_duration_cast<To, FromRep, FromPeriod>(from, ec);
  if (ec) {
    throw std::runtime_error("failed conversion");
  }
  return ret;
} // func
#endif
} // namespace safe_duration_cast
