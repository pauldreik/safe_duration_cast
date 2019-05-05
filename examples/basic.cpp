#include <cassert>
#include <chrono>
#include <iostream>
#include <limits>
#include <stdexcept>
#include <type_traits>

#include <detail/lossless_conversion.hpp>

namespace safe_duration_cast {

namespace detail {
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


template<typename To, typename From>
constexpr To
duration_cast_impl(From from, int& ec)
{
  // the basic idea is that we need to convert from count() in the from type
  // to count() in the To type, by multiplying it with this:
  using Factor = std::ratio_divide<typename From::period, typename To::period>;

  static_assert(Factor::num > 0, "num must be positive");
  static_assert(Factor::den > 0, "den must be positive");

  // the conversion is like this: multiply from.count() with Factor::num
  // /Factor::den and convert it to To::rep, all this without
  // overflow/underflow. let's start by finding a suitable type that can hold
  // both To, From and Factor::num
  using IntermediateRep = std::
    common_type<typename From::rep, typename To::rep, decltype(Factor::num)>;
  // todo - safe conversion to IntermediateRep
  IntermediateRep count = from.count();
  // todo - make sure this does not overflow
  count *= Factor::num;
  // this can't go wrong, right? den>0 is checked earlier.
  count /= Factor::den;
  // convert to the to type, safely
  using ToRep = typename From::rep;
  const ToRep tocount = count;
  return To{ tocount };
}

} // detail

/**
 * converts From to To (potentially lossy, just like std::chrono::duration_cast,
 * but avoids overflow
 */
template<typename To, typename From>
constexpr To
safe_duration_cast(From from, int& ec)
{
  static_assert(detail::is_duration(From{}), "From is not a duration");
  static_assert(detail::is_duration(To{}), "To is not a duration");

  const auto to = std::chrono::duration_cast<To>(from);
  return to;
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

struct NotADuration
{};

template<typename To, typename From>
void
doit(const From& from)
{
  std::cout << "From.count()=" << from.count() << '\t';
  To to = safe_duration_cast::safe_duration_cast<To>(from);
  std::cout << "To.count()=" << to.count() << '\n';
}

int
main(int argc, char* argv[])
{
  using From = std::chrono::duration<int, std::milli>;
  using To = std::chrono::duration<int, std::deci>;

  // auto to=std::chrono::duration_cast<To>(From{1});
  const auto to = safe_duration_cast::safe_duration_cast<To>(From{ 1 });
  // safe_duration_cast::safe_duration_cast<NotADuration>(From{ 1 });

  doit<std::chrono::milliseconds>(std::chrono::seconds{ 23 });
  doit<std::chrono::seconds>(std::chrono::milliseconds{ 23 });
}
