/*
 * By Paul Dreik 2019
 *
 * License:
 * dual license, pick your choice. Either Boost license 1.0, or GPL(v2 or later,
 * at your option).
 */
#include <safe_duration_cast/chronoconv.hpp>
#include <cstdint>
#include <cstring>
#include <stdexcept>
#include <type_traits>
#include <vector>

#include <boost/multiprecision/cpp_bin_float.hpp>
#include <cfenv>
#include <tuple>
#include <utility>

// for finding floating point exceptions quick and nailing
// down where bad conversions happen
#define CHECK_FP_EXCEPTION assert(std::fetestexcept(FE_INVALID) == 0)

namespace foreach_detail {
template<class Tup, class Func, std::size_t... Is>
constexpr void
static_foreach_impl(const Tup& t, Func&& f, std::index_sequence<Is...>)
{
  (f(std::integral_constant<std::size_t, Is>{}, std::get<Is>(t)), ...);
}
}

/**
 * Applies f on each element of t, in term.
 * @param t
 * @param f
 */
template<class... T, class Func>
constexpr void
static_foreach(const std::tuple<T...>& t, Func&& f)
{
  constexpr auto tuple_size = sizeof...(T);
  foreach_detail::static_foreach_impl(
    t, std::forward<Func>(f), std::make_index_sequence<tuple_size>{});
}
auto
makeLargeBoostFloat()
{
  using namespace boost::multiprecision;
  return number<backends::cpp_bin_float<237,
                                        backends::digit_base_2,
                                        void,
                                        boost::int32_t,
                                        -262142,
                                        262143>,
                et_off>{};
}
template<typename FromRep, typename Ratio, typename ToRep, typename RatioTo>
void
use_different_rep(const FromRep item)
{
  CHECK_FP_EXCEPTION;
  using From = std::chrono::duration<FromRep, Ratio>;
  const From from{ item };
  using To = std::chrono::duration<ToRep, RatioTo>;

  CHECK_FP_EXCEPTION;
  int ec;
  const auto to = safe_duration_cast::safe_duration_cast<To>(from, ec);
  CHECK_FP_EXCEPTION;

  if (ec) {
    // could not convert, fine
    return;
  }
  CHECK_FP_EXCEPTION;

  if (std::isinf(from.count())) {
    assert(std::isinf(to.count()));
    return;
  }
  std::feclearexcept(FE_ALL_EXCEPT);
  CHECK_FP_EXCEPTION;

  // could convert. let's check the result is as expected
  using LargeFloat = decltype(
    makeLargeBoostFloat()); // boost::multiprecision::cpp_bin_float_double_extended;
  using K = typename std::ratio_divide<Ratio, RatioTo>::type;
  const LargeFloat b = [=]() {
    CHECK_FP_EXCEPTION;
    LargeFloat tmp{ item };
    // seems like this sets exceptions. clear them.
    std::feclearexcept(FE_ALL_EXCEPT);
    CHECK_FP_EXCEPTION;
    tmp *= LargeFloat{ K::num };
    CHECK_FP_EXCEPTION;
    tmp /= LargeFloat{ K::den };
    CHECK_FP_EXCEPTION;
    return tmp;
  }();

  const LargeFloat absdiff = b - LargeFloat{ to.count() };
  CHECK_FP_EXCEPTION;
  const LargeFloat absb = b < 0 ? -b : b; // std::abs(b);
  CHECK_FP_EXCEPTION;
  // if we are finite and above the subnormal range, do a relative check of
  // the result
  if (absb > std::numeric_limits<FromRep>::min() &&
      absb > std::numeric_limits<ToRep>::min()) {
    const LargeFloat reldiff = absdiff / absb;
    CHECK_FP_EXCEPTION;
    const auto tol1 = std::numeric_limits<FromRep>::epsilon() * 2;
    const auto tol2 = std::numeric_limits<ToRep>::epsilon() * 2;
    using TolType = long double;
    const auto tol = std::max(TolType{ tol1 }, TolType{ tol2 });
    CHECK_FP_EXCEPTION;
    if (!(reldiff < tol)) {
      CHECK_FP_EXCEPTION;
      std::cout << "from=" << from.count() << " to=" << to.count() << " b=" << b
                << " absdiff=" << absdiff << " reldiff=" << reldiff
                << " tol=" << tol << '\n';
    }
    assert(reldiff < tol);
  }
  CHECK_FP_EXCEPTION;
}

// Item is the underlying type for duration (int, long etc)
template<typename Item1, typename Item2>
void
doit(const uint8_t* Data, std::size_t Size)
{
  const auto N1 = sizeof(Item1);
  if (Size < N1) {
    return;
  }
  Item1 item1{};
  std::memcpy(&item1, Data, N1);
  Data += N1;
  Size -= N1;
  using ratios = std::tuple<std::milli, std::kilo>;
  static_foreach(ratios{}, [=](auto /*i*/, auto a) {
    using RatioFrom = decltype(a);
    static_foreach(ratios{}, [=](auto /*i*/, auto b) {
      using RatioTo = decltype(b);
      use_different_rep<Item1, RatioFrom, Item2, RatioTo>(item1);
    });
  });
}

extern "C" int
LLVMFuzzerTestOneInput(const uint8_t* Data, std::size_t Size)
{
  // all initialization should happen only once per fuzzing session
  static bool isinit = []() {
    std::feclearexcept(FE_ALL_EXCEPT);
    return true;
  }();
  if (Size <= 4) {
    return 0;
  }

  // do a dynamic dispatch to a static type combination. Yay!
  const auto firsttype = Data[0];
  Data++;
  Size--;
  const auto secondtype = Data[0];
  Data++;
  Size--;

  using fundamental_ints = std::tuple<float, double, long double>;

  static_foreach(fundamental_ints{}, [=](auto i, auto a) {
    if (i == firsttype) {
      static_foreach(fundamental_ints{}, [=](auto j, auto b) {
        if (j == secondtype) {
          CHECK_FP_EXCEPTION;

          using A = std::decay_t<decltype(a)>;
          using B = std::decay_t<decltype(b)>;
          doit<A, B>(Data, Size);
          CHECK_FP_EXCEPTION;
        }
      });
    }
  });
  CHECK_FP_EXCEPTION;
  return 0;
}

#ifdef IMPLEMENT_MAIN
#include <cassert>
#include <fstream>
#include <sstream>
#include <vector>
int
main(int argc, char* argv[])
{
  for (int i = 1; i < argc; ++i) {
    std::ifstream in(argv[i]);
    assert(in);
    in.seekg(0, std::ios_base::end);
    const auto pos = in.tellg();
    in.seekg(0, std::ios_base::beg);
    std::vector<char> buf(pos);
    in.read(buf.data(), buf.size());
    assert(in.gcount() == pos);
    LLVMFuzzerTestOneInput((const uint8_t*)buf.data(), buf.size());
  }
}
#endif
