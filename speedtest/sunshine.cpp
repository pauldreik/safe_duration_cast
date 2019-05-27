/*
 * By Paul Dreik 2019
 *
 * License:
 * dual license, pick your choice. Either Boost license 1.0, or GPL(v2 or later,
 * at your option).
 */

#include "chronoconv.hpp"

#include <iostream>
#include <cassert>
#include <cstring>
#include <limits>

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

private:
  //assumes clang or gcc.
  __uint128_t m_state;
};

int
main(int /*argc*/, char* argv[])
{
  // initialize the rng to something the compiler cant know
  Lehmer rng(argv[0], std::strlen(argv[0]));

  using From = std::chrono::duration<std::uint64_t>;
  using To = std::chrono::duration<std::uint64_t, std::ratio<3, 5>>;

  const auto t0 = std::chrono::steady_clock::now();

  std::uint64_t dummycount = 0;
  const auto iterations = 400000000U;
  for (unsigned int i = 0; i < iterations; ++i) {
    const auto input = rng.get();
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
