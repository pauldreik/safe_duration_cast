/*
 * By Paul Dreik 2019
 *
 * License:
 * dual license, pick your choice. Either Boost license 1.0, or GPL(v2 or later,
 * at your option).
 *
 * exhaustive tests for float types, to cover all 2^32 possible
 * float values, validating them against std::chrono:duration_cast.
 */
#include <iostream>

#include "chronoconv.hpp"

#include <cassert>
#include <cmath>
#include <cstring>
#include <future>
#include <limits>
#include <thread>
#include <vector>
#include <cfenv>

using Count = std::int64_t;

struct Outcome
{
  Count passed = 0;
  Count problematic = 0;
};

template<class To, class ToPeriod>
Outcome
testAll(const unsigned threadIndex, const unsigned Nthreads)
{
  assert(threadIndex < Nthreads);
  using LoopVar = std::uint32_t;
  using From = float;
  static_assert(sizeof(LoopVar) == sizeof(From), "size assumption");

  Outcome ret;
  auto body = [&ret](const LoopVar f) {
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
  };

  // execute the loop body once for each possible value and
  // also not overflowing the loop variable
  const auto min = std::numeric_limits<LoopVar>::min();
  const auto max = std::numeric_limits<LoopVar>::max();
  const auto blocksize = (std::uint64_t{ max } - min + 1) / Nthreads;
  const auto begin = threadIndex * blocksize + min;
  const auto beforeend = [=]() {
    if (threadIndex + 1 == Nthreads) {
      return max;
    } else {
      return static_cast<LoopVar>(begin + blocksize - 1);
    }
  }();
  for (LoopVar f = static_cast<LoopVar>(begin); f < beforeend; ++f) {
    body(f);
  }
  body(beforeend);

  return ret;
}

template<class To, class ToPeriod>
Outcome
runThreaded(const unsigned Nthreads)
{
  std::vector<std::future<Outcome>> results(Nthreads);

  for (unsigned i = 0; i < Nthreads; ++i) {
    results[i] = std::async(
      std::launch::async, [=]() { return testAll<To, ToPeriod>(i, Nthreads); });
  }
  Outcome sum;
  for (unsigned i = 0; i < Nthreads; ++i) {
    const auto Partial = results[i].get();
    sum.problematic += Partial.problematic;
    sum.passed += Partial.passed;
  }
  std::cout << __PRETTY_FUNCTION__ << " problematic=" << sum.problematic
            << "\tpassed=" << sum.passed << std::endl;

  return sum;
}

int
main()
{
  const auto nthreads = std::thread::hardware_concurrency();
  runThreaded<float, std::ratio<3, 5>>(nthreads);
  runThreaded<float, std::ratio<1, 1>>(nthreads);
  runThreaded<float, std::ratio<5, 3>>(nthreads);
  runThreaded<double, std::ratio<3, 5>>(nthreads);
  runThreaded<double, std::ratio<1, 1>>(nthreads);
  runThreaded<double, std::ratio<5, 3>>(nthreads);
  return 0;
}
