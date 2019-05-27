/*
 * By Paul Dreik 2019
 *
 * License:
 * dual license, pick your choice. Either Boost license 1.0, or GPL(v2 or later,
 * at your option).
 */
#pragma once


#include <cstring>
#include <limits>

// fast rng. see this blog post
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

