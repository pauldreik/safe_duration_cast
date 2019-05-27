/*
 * By Paul Dreik 2019
 *
 * License:
 * dual license, pick your choice. Either Boost license 1.0, or GPL(v2 or later,
 * at your option).
 */

#include "chronoconv.hpp"

#include <cassert>
#include <cstring>
#include <iostream>
#include <limits>
#include <random>

// fast rng. see
// https://lemire.me/blog/2019/03/19/the-fastest-conventional-random-number-generator-that-can-pass-big-crush/
class Lehmer
{
public:
  Lehmer(const char* seed, std::size_t Len)
  {
    m_state = 0;
    std::memcpy(&m_state, seed, std::min(Len, sizeof(m_state)));
  }

  std::uint64_t get()
  {
    m_state *= 0xda942042e4dd58b5;
    return m_state >> 64;
  }

  // lets make it fulfill
  // https://en.cppreference.com/w/cpp/named_req/UniformRandomBitGenerator
  using result_type = std::uint64_t;
  constexpr static std::uint64_t min() { return 0; }
  constexpr static std::uint64_t max()
  {
    return std::numeric_limits<std::uint64_t>::max();
  }
  std::uint64_t operator()() { return get(); }

private:
  // assumes clang or gcc.
  __uint128_t m_state;
};

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

int
main(int /*argc*/, char* argv[])
{
  using From = std::chrono::duration<std::uint64_t>;
  using To = std::chrono::duration<std::uint64_t, std::ratio<3, 5>>;

  constexpr auto minsafe = findLowestNonproblematicInput<From, To>();
  constexpr auto maxsafe = findLargestNonproblematicInput<From, To>();
  std::cout << "safe input range is " << minsafe.count() << " to "
            << maxsafe.count() << '\n';

  // initialize the rng to something the compiler cant know
  Lehmer rng(argv[0], std::strlen(argv[0]));

  std::uniform_int_distribution<std::uint64_t> dist(minsafe.count(),
                                                    maxsafe.count());

  const auto t0 = std::chrono::steady_clock::now();

  std::uint64_t dummycount = 0;
  const auto iterations = 400000000U;
  for (unsigned int i = 0; i < iterations; ++i) {
    const auto input = dist(rng); // rng.get();
    int ec;
    const auto from = From{ input };
    const auto result = safe_duration_cast::safe_duration_cast<To>(from, ec);
    if (ec == 0) {
      dummycount += result.count() & 0x1;
    }
  }
  const auto t1 = std::chrono::steady_clock::now();
  const auto elapsed_seconds =
    std::chrono::duration_cast<std::chrono::duration<double>>(t1 - t0).count();
  std::cout << "speed: " << iterations / elapsed_seconds
            << " operations per second, dummy=" << dummycount << "\n";
}
