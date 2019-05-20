/*
 * By Paul Dreik 2019
 *
 * License:
 * dual license, pick your choice. Either Boost license 1.0, or GPL(v2 or later,
 * at your option).
 */
#include <cassert>
#include <chrono>
#include <chronoconv.hpp>
#include <climits>
#include <iostream>
#include <limits>
#include <stdexcept>
#include <type_traits>

void
example1_wrong_result()
{
  using mSec = std::chrono::duration<int, std::milli>;
  using Sec = std::chrono::duration<int>;

  // converting a large number of seconds to milliseconds wont fit
  // in the target type, so this eventually ends up in implementation defined
  // behaviour and it gives the wrong result.
  const Sec from{ INT_MAX / 1000 + 1 }; // change +1 to -1, and it works
  const mSec to = from;
  assert(to.count() / 1000 == from.count() && "oops, wrong answer!");
}

void
example2_undefined_behaviour()
{
  using mSec = std::chrono::duration<long long, std::milli>;
  using Sec = std::chrono::duration<long long>;

  // this will cause an internal signed overflow inside chrono, which is
  // undefined behaviour. Here is what ubsan says:
  // /usr/include/c++/7/chrono:176:38: runtime error: signed integer overflow:
  // 9223372036854776 * 1000 cannot be represented in type 'long long int'

  const Sec from{ LLONG_MAX / 1000 + 1 };
  const mSec to = from;
  assert(to.count() / 1000 == from.count() && "oops, wrong answer!");
}

int
main(int argc, char* argv[])
{
  //  example1_wrong_result();
  example2_undefined_behaviour();
}
