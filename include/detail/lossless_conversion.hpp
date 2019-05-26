/*
 * By Paul Dreik 2019
 *
 * License:
 * dual license, pick your choice. Either Boost license 1.0, or GPL(v2 or later,
 * at your option).
 */
#ifndef INCLUDE_DETAIL_LOSSLESS_CONVERSION_HPP_
#define INCLUDE_DETAIL_LOSSLESS_CONVERSION_HPP_

#include <limits>

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
namespace safe_duration_cast {

/**
 * converts From to To, without loss. If the dynamic value of from
 * can't be converted to To without loss, ec is set.
 */
template<typename To, typename From>
SDC_RELAXED_CONSTEXPR To
lossless_integral_conversion(From from, int& ec)
{
  ec = 0;
  using F = std::numeric_limits<From>;
  using T = std::numeric_limits<To>;
  static_assert(F::is_integer, "From must be integral");
  static_assert(T::is_integer, "To must be integral");

  if
    SDC_CONSTEXPR_IF(F::is_signed == T::is_signed)
    {
      // A and B are both signed, or both unsigned.
      if
        SDC_CONSTEXPR_IF(F::digits <= T::digits)
        {
          // From fits in To without any problem
        }
      else {
        // From does not always fit in To, resort to a dynamic check.
        if (from < T::min() || from > T::max()) {
          // outside range.
          ec = 1;
          return {};
        }
      }
    }

  if
    SDC_CONSTEXPR_IF(F::is_signed && !T::is_signed)
    {
      // From may be negative, not allowed!
      if (from < 0) {
        ec = 1;
        return {};
      }

      // From is positive. Can it always fit in To?
      if
        SDC_CONSTEXPR_IF(F::digits <= T::digits)
        {
          // yes, From always fits in To.
        }
      else {
        // from may not fit in To, we have to do a dynamic check
        if (from > T::max()) {
          ec = 1;
          return {};
        }
      }
    }

  if
    SDC_CONSTEXPR_IF(!F::is_signed && T::is_signed)
    {
      // can from be held in To?
      if
        SDC_CONSTEXPR_IF(F::digits < T::digits)
        {
          // yes, From always fits in To.
        }
      else {
        // from may not fit in To, we have to do a dynamic check
        if (from > T::max()) {
          // outside range.
          ec = 1;
          return {};
        }
      }
    }

  // reaching here means all is ok for lossless conversion.
  return static_cast<To>(from);

} // function

} // namespace
#endif /* INCLUDE_DETAIL_LOSSLESS_CONVERSION_HPP_ */
