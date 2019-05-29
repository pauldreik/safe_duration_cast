/*
 * By Paul Dreik 2019
 *
 * License:
 * dual license, pick your choice. Either Boost license 1.0, or GPL(v2 or later,
 * at your option).
 */

#include "safe_duration_cast/chronoconv.hpp"

#include <algorithm>
#include <cassert>
#include <cstring>
#include <iostream>
#include <limits>

/**
 * finds the largest input that can be converted from FromDuration to ToDuration
 * without danger.
 */
template<class FromDuration, class ToDuration>
constexpr FromDuration
findLargestNonproblematicInput()
{
  auto worksfine = [](const typename FromDuration::rep value) /*constexpr*/ {
    int ec = 0;
    const auto from = FromDuration{ value };
    /*const auto result =*/safe_duration_cast::safe_duration_cast<ToDuration>(
      from, ec);
    return ec == 0;
  };
  typename FromDuration::rep min = 0;
  typename FromDuration::rep max =
    std::numeric_limits<typename FromDuration::rep>::max();

  // assert(worksfine(min));
  if (worksfine(max)) {
    return FromDuration{ max };
  }
  while ((max - min) > 1) {
    auto candidate = min + (max - min) / 2;
    // std::cout<<"min="<<min<<" max="<<max<<" trying "<<candidate<<'\n';
    if (worksfine(candidate)) {
      min = candidate;
    } else {
      max = candidate;
    }
  }
  return FromDuration{ min };
}

template<class FromDuration, class ToDuration>
constexpr FromDuration
findLowestNonproblematicInput()
{
  auto worksfine = [](const typename FromDuration::rep value) /*constexpr*/ {
    int ec = 0;
    const auto from = FromDuration{ value };
    /*const auto result =*/safe_duration_cast::safe_duration_cast<ToDuration>(
      from, ec);
    return ec == 0;
  };
  typename FromDuration::rep min =
    std::numeric_limits<typename FromDuration::rep>::lowest();
  typename FromDuration::rep max =
    0; // std::numeric_limits<typename FromDuration::rep>::max();

  // assert(worksfine(max));
  if (worksfine(min)) {
    return FromDuration{ min };
  }
  while ((max - min) > 1) {
    auto candidate = min + (max - min) / 2;
    // std::cout<<"min="<<min<<" max="<<max<<" trying "<<candidate<<'\n';
    if (worksfine(candidate)) {
      max = candidate;
    } else {
      min = candidate;
    }
  }
  return FromDuration{ max };
}

template<bool usestdchrono>
int
doit(int /*argc*/, char* argv[])
{

  using From = std::chrono::duration<std::uint64_t>;
  using To = std::chrono::duration<std::uint64_t, std::ratio<3, 5>>;

  constexpr auto minsafe = findLowestNonproblematicInput<From, To>();
  constexpr auto maxsafe = findLargestNonproblematicInput<From, To>();
  /*
  std::cout << "safe input range is " << minsafe.count() << " to "
            << maxsafe.count() << '\n';
*/

  const auto t0 = std::chrono::steady_clock::now();

  std::uint64_t dummycount = 0;
  const std::uint64_t iterations =
    std::min(std::uint64_t{ 2000000000ULL }, maxsafe.count());
  for (std::uint64_t i = 0; i < iterations; ++i) {
    const std::uint64_t input = i;
    int ec;
    const auto from = From{ input };
    if (!usestdchrono) {
      const auto result = safe_duration_cast::safe_duration_cast<To>(from, ec);
      dummycount += result.count();
    } else {
      const auto result = std::chrono::duration_cast<To>(from);
      dummycount += result.count();
    }
  }
  const auto t1 = std::chrono::steady_clock::now();
  const auto elapsed_seconds =
    std::chrono::duration_cast<std::chrono::duration<double>>(t1 - t0).count();
  std::cout << (usestdchrono ? "std::chrono::duration_cast"
                             : "safe_duration_cast")
            << " speed:\t" << iterations / elapsed_seconds
            << " operations per second, dummy=" << dummycount << "\n";
  return 0;
}

int
main(int argc, char* argv[])
{
  doit<false>(argc, argv);
  doit<true>(argc, argv);
  doit<false>(argc, argv);
  doit<true>(argc, argv);
  doit<false>(argc, argv);
  doit<true>(argc, argv);
}
