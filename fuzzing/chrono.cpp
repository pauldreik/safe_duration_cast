// Copyright (c) 2019, Paul Dreik

#include <chronoconv.hpp>
#include <cstdint>
#include <cstring>
#include <stdexcept>
#include <type_traits>
#include <vector>

#include <tuple>
#include <utility>

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

template<typename Item, typename Ratio, typename ToRep>
void
use_different_rep(const Item item)
{
  using From = std::chrono::duration<Item, Ratio>;
  const From from{ item };

  using DoubleRatio = std::ratio_multiply<Ratio, std::ratio<2, 1>>;
  using ToDouble = std::chrono::duration<ToRep, DoubleRatio>;

  // this should fail or succeed, either way without undefined behaviour.
  int ecdouble;
  const auto todouble =
    safe_duration_cast::safe_duration_cast<ToDouble>(from, ecdouble);

  // if the representation is unchanged, we should be able to check that
  // the results are ok.
  if constexpr (std::is_same<Item, ToRep>::value) {
    assert(ecdouble == 0);
    assert(item / 2 == todouble.count());
  }

  using HalfRatio = std::ratio_multiply<Ratio, std::ratio<1, 2>>;
  using ToHalf = std::chrono::duration<ToRep, HalfRatio>;

  // this should fail or succeed, either way without undefined behaviour.
  int echalf;
  const auto tohalf =
    safe_duration_cast::safe_duration_cast<ToHalf>(from, echalf);

  // if the representation is unchanged, we should be able to check that
  // the results are ok.
  if constexpr (std::is_same<Item, ToRep>::value) {
    if (echalf != 0) {
      // value would not fit.
      return;
    }
    // make sure the value is what we expect it to be
    assert(tohalf.count() / 2 == item);
  }
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
    using R = decltype(a);
    use_different_rep<Item1, R, Item2>(item1);
  });
  // use_same_rep<Item,std::yocto>(item);
  // use_same_rep<Item,std::zepto>(item);

  use_different_rep<Item1, std::kilo, Item2>(item1);
  // use_same_rep<Item,std::zeta>(item);
  // use_same_rep<Item,std::yotta>(item);
}

extern "C" int
LLVMFuzzerTestOneInput(const uint8_t* Data, std::size_t Size)
{
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

  using fundamental_ints =
    std::tuple<char, short, int, long, long long, std::intmax_t>;

  static_foreach(fundamental_ints{}, [=](auto i, auto a) {
    if (i == firsttype) {
      static_foreach(fundamental_ints{}, [=](auto j, auto b) {
        if (j == secondtype) {
          using A = std::decay_t<decltype(a)>;
          using B = std::decay_t<decltype(b)>;
          using sA = typename std::make_signed<A>::type;
          using sB = typename std::make_signed<B>::type;
          using uA = typename std::make_unsigned<A>::type;
          using uB = typename std::make_unsigned<B>::type;
          doit<sA, sB>(Data, Size);
          doit<sA, uB>(Data, Size);
          if constexpr (!std::is_same<sA, sB>::value) {
            doit<uA, sB>(Data, Size);
            doit<uA, uB>(Data, Size);
          }
        }
      });
    }
  });
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
