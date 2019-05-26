/*
 * By Paul Dreik 2019
 *
 * License:
 * dual license, pick your choice. Either Boost license 1.0, or GPL(v2 or later,
 * at your option).
 */
#include <cassert>
#include <cfenv>
#include <chrono>
#include <cmath>
#include <iostream>
#include <limits>
#include <stdexcept>
#include <type_traits>

#include <detail/lossless_conversion.hpp>
#include <detail/safe_float_conversion.hpp>
#include <detail/stdutils.hpp>

namespace safe_duration_cast {

namespace detail {

namespace tags {
struct FromIsInt
{};
struct ToIsInt
{};
struct FromIsFloat
{};
struct ToIsFloat
{};
struct NotArithmetic
{};
} // namespace tags

// like std::conditional, but with an extra condition
// if B1
//   type is T
// elseif B2
//   type is E
// else
//   type is F
template<bool B1, class T, bool B2, class E, class F>
struct conditional3 : std::conditional<B2, E, F>
{};

template<class T, bool B2, class E, class F>
struct conditional3<true, T, B2, E, F>
{
  using type = T;
};

template<typename Rep, typename Period>
constexpr bool is_duration(std::chrono::duration<Rep, Period>)
{
  return true;
}
constexpr bool
is_duration(...)
{
  return false;
}
template<typename Rep, typename Period>
constexpr bool is_integral_duration(std::chrono::duration<Rep, Period>)
{
  return std::is_integral<Rep>::value;
}
constexpr bool
is_integral_duration(...)
{
  return false;
}
template<typename Rep, typename Period>
constexpr bool is_floating_duration(std::chrono::duration<Rep, Period>)
{
  return std::is_floating_point<Rep>::value;
}
constexpr bool
is_floating_duration(...)
{
  return false;
}

template<typename To, typename From>
SDC_RELAXED_CONSTEXPR To
safe_duration_cast_dispatch(From from, int& ec, tags::FromIsInt, tags::ToIsInt)
{
  static_assert(is_integral_duration(From{}), "from must be integral");
  static_assert(is_integral_duration(To{}), "to must be integral");
  ec = 0;
  // the basic idea is that we need to convert from count() in the from type
  // to count() in the To type, by multiplying it with this:
  using Factor = std::ratio_divide<typename From::period, typename To::period>;

  static_assert(Factor::num > 0, "num must be positive");
  static_assert(Factor::den > 0, "den must be positive");

  // the conversion is like this: multiply from.count() with Factor::num
  // /Factor::den and convert it to To::rep, all this without
  // overflow/underflow. let's start by finding a suitable type that can hold
  // both To, From and Factor::num
  using IntermediateRep =
    typename std::common_type<typename From::rep,
                              typename To::rep,
                              decltype(Factor::num)>::type;

  // safe conversion to IntermediateRep
  IntermediateRep count =
    lossless_integral_conversion<IntermediateRep>(from.count(), ec);
  if (ec) {
    return {};
  }
  // multiply with Factor::num without overflow or underflow
  constexpr auto max1 =
    std::numeric_limits<IntermediateRep>::max() / Factor::num;
  if (count > max1) {
    ec = 1;
    return {};
  }
  constexpr auto min1 =
    std::numeric_limits<IntermediateRep>::min() / Factor::num;
  if (count < min1) {
    ec = 1;
    return {};
  }
  count *= Factor::num;

  // this can't go wrong, right? den>0 is checked earlier.
  count /= Factor::den;
  // convert to the to type, safely
  using ToRep = typename To::rep;
  const ToRep tocount = lossless_integral_conversion<ToRep>(count, ec);
  if (ec) {
    return {};
  }
  return To{ tocount };
}

// converts From to To, asserting no floating point exceptions
// have happened.
template<typename To, typename From>
To
convert_and_check_cfenv(From from)
{
  assert(std::fetestexcept(FE_INVALID) == 0);
  static_assert(std::is_floating_point<From>::value, "");
  To count = from;
  if
    SDC_CONSTEXPR_IF(std::is_floating_point<To>::value)
    {
      // conversion float -> float
      if (std::fetestexcept(FE_INVALID) != 0) {
        std::cout << "From=" << from << '\n';
      }
      assert(std::fetestexcept(FE_INVALID) == 0);
    }
  else {
    // conversion float->integer
    assert(std::fetestexcept(FE_INVALID) == 0);
  }
  return count;
}

template<typename To, typename From>
SDC_RELAXED_CONSTEXPR To
safe_duration_cast_dispatch(From from,
                            int& ec,
                            tags::FromIsFloat,
                            tags::ToIsFloat)
{
  static_assert(is_floating_duration(From{}), "from must be floating point");
  static_assert(is_floating_duration(To{}), "to must be floating point");
  assert(std::fetestexcept(FE_INVALID) == 0);
  if (std::isnan(from.count())) {
    // seems like the use of isnan raises an exception in itself!
    std::feclearexcept(FE_ALL_EXCEPT);
    assert(std::fetestexcept(FE_INVALID) == 0);
    // nan in, gives nan out. easy.
    return To{ std::numeric_limits<typename To::rep>::quiet_NaN() };
  }
  // maybe we should also check if from is denormal, and decide what to do about
  // it.

  // +-inf should be preserved.
  if (std::isinf(from.count())) {
    return To{ from.count() };
  }

  assert(std::fetestexcept(FE_INVALID) == 0);

  ec = 0;
  // the basic idea is that we need to convert from count() in the from type
  // to count() in the To type, by multiplying it with this:
  using Factor = std::ratio_divide<typename From::period, typename To::period>;

  static_assert(Factor::num > 0, "num must be positive");
  static_assert(Factor::den > 0, "den must be positive");

  // the conversion is like this: multiply from.count() with Factor::num
  // /Factor::den and convert it to To::rep, all this without
  // overflow/underflow. let's start by finding a suitable type that can hold
  // both To, From and Factor::num
  using IntermediateRep =
    typename std::common_type<typename From::rep,
                              typename To::rep,
                              decltype(Factor::num)>::type;

  // check this in a template, to get type info in the error message
  IntermediateRep count =
    convert_and_check_cfenv<IntermediateRep>(from.count());

  // multiply with Factor::num without overflow or underflow
  constexpr auto max1 =
    std::numeric_limits<IntermediateRep>::max() / Factor::num;
  if (count > max1) {
    ec = 1;
    return {};
  }
  constexpr auto min1 =
    std::numeric_limits<IntermediateRep>::lowest() / Factor::num;
  if (count < min1) {
    ec = 1;
    return {};
  }
  count *= Factor::num;
  assert(std::fetestexcept(FE_INVALID) == 0);

  // this can't go wrong, right? den>0 is checked earlier.
  count /= Factor::den;
  // convert to the to type, safely
  using ToRep = typename To::rep;

  const ToRep tocount = safe_float_conversion<ToRep>(count, ec);
  if (ec) {
    return {};
  }
  assert(std::fetestexcept(FE_INVALID) == 0);
  return To{ tocount };
}
} // detail
} // namespace safe_duration_cast
