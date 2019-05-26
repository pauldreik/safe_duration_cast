#include <iostream>

#include "chronoconv.hpp"

#include <cassert>
#include <cmath>
#include <cstring>
#include <limits>

using Count = std::int64_t;

struct Outcome
{
  Count passed = 0;
  Count problematic = 0;
};

template<class To, class ToPeriod>
Outcome
testAll()
{
  using LoopVar = std::uint32_t;
  using From = float;
  static_assert(sizeof(LoopVar) == sizeof(From), "size assumption");

  const auto min = std::numeric_limits<LoopVar>::min();
  const auto max = std::numeric_limits<LoopVar>::max();
  Outcome ret;
  for (LoopVar f = min; f != max; ++f) {
    int ec = 0;
    using FromDur = std::chrono::duration<From>;
    using ToDur = std::chrono::duration<To, ToPeriod>;
    float tmp;
    std::memcpy(&tmp, &f, sizeof(f));
    const FromDur from{ tmp };
    std::feclearexcept(FE_ALL_EXCEPT);
    const auto to = safe_duration_cast::safe_duration_cast<ToDur>(from, ec);
    if (ec == 0) {
      const auto ref = std::chrono::duration_cast<ToDur>(from);
      if (to != ref) {
        // we come here if to!=ref, or any of them is NaN. if any is NaN, the
        // other had better be it too
        if (std::isnan(to.count()) && std::isnan(ref.count())) {
          // all is fine.
        } else {
          std::cout << "failed test in " << __PRETTY_FUNCTION__
                    << ": loopvar=" << f << "=" << tmp << " to=" << to.count()
                    << " ref=" << ref.count() << std::endl;
          std::abort();
        }
      }
      ++ret.passed;
    } else {
      ++ret.problematic;
    }
  }
  std::cout << __PRETTY_FUNCTION__ << " problematic=" << ret.problematic
            << "\tpassed=" << ret.passed << std::endl;
  return ret;
}

int
main()
{
  std::cout << "Hello World!" << __cplusplus << '\n';
  testAll<float, std::ratio<3, 5>>();
  testAll<float, std::ratio<1, 1>>();
  testAll<float, std::ratio<5, 3>>();
  testAll<double, std::ratio<3, 5>>();
  testAll<double, std::ratio<1, 1>>();
  testAll<double, std::ratio<5, 3>>();
  return 0;
}
