#include <iostream>

#include "chronoconv.hpp"

#include <cassert>
#include <limits>

using Count = std::int64_t;

struct Outcome
{
  Count passed = 0;
  Count problematic = 0;
};

template<class From, class To>
Outcome
testAll()
{
  static_assert(sizeof(From) <= 4, "don't want to wait that long");

  const auto min = std::numeric_limits<From>::min();
  const auto max = std::numeric_limits<From>::max();
  Outcome ret;
  for (From f = min; f != max; ++f) {
    int ec = 0;
    using FromDur = std::chrono::duration<From>;
    using ToDur = std::chrono::duration<To>;
    const FromDur from{ f };
    const auto to = safe_duration_cast::safe_duration_cast<ToDur>(from, ec);
    if (ec == 0) {
      const auto ref = std::chrono::duration_cast<ToDur>(from);
      assert(to == ref);
      ++ret.passed;
    } else {
      ++ret.problematic;
    }
  }
  return ret;
}
template<class From, class To>
Outcome
testAllAndPrint()
{
  const auto ret = testAll<From, To>();
  std::cout << __PRETTY_FUNCTION__ << " problematic=" << ret.problematic
            << "\tpassed=" << ret.passed << std::endl;
  return ret;
}

template<class F>
void
onEach(F&& f)
{
  f(char{});
  using uc = unsigned char;
  f(uc{});
  using sc = signed char;
  f(sc{});
  f(short{});
  using us = unsigned short;
  f(us{});
  f(int{});
  return;
  f(unsigned{});
}

int
main()
{
  std::cout << "Hello World!" << __cplusplus << '\n';
  onEach([](auto dummy1) {
    onEach([](auto dummy2) {
      using D1 = decltype(dummy1);
      using D2 = decltype(dummy2);
      if (!std::is_same<D1, D2>()) {
        testAllAndPrint<D1, D2>();
      }
    });
  });
  return 0;
}
