// Copyright (c) 2019, Paul Dreik

#include <chronoconv.hpp>
#include <cstdint>
#include <cstring>
#include <stdexcept>
#include <type_traits>
#include <vector>

#include <cfenv>
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
  using To = std::chrono::duration<ToRep, Ratio>;

  int ec;
  const auto to = safe_duration_cast::safe_duration_cast<To>(from, ec);
  assert(std::fetestexcept(FE_INVALID) == 0);
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
}

extern "C" int
LLVMFuzzerTestOneInput(const uint8_t* Data, std::size_t Size)
{
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
          assert(std::fetestexcept(FE_INVALID) == 0);

          using A = std::decay_t<decltype(a)>;
          using B = std::decay_t<decltype(b)>;
          doit<A, B>(Data, Size);
          assert(std::fetestexcept(FE_INVALID) == 0);
        }
      });
    }
  });
  assert(std::fetestexcept(FE_INVALID) == 0);
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
