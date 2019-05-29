/*
 * By Paul Dreik 2019
 *
 * License:
 * dual license, pick your choice. Either Boost license 1.0, or GPL(v2 or later,
 * at your option).
 */
#include <cassert>
#include <cfloat>
#include <chrono>
#include <safe_duration_cast/chronoconv.hpp>
#include <climits>
#include <iostream>
#include <limits>
#include <stdexcept>
#include <type_traits>

void
example1_wrong_result_after_overflow()
{
  using Sec = std::chrono::duration<int>;
  using mSec = std::chrono::duration<int, std::milli>;

  // converting a large number of seconds to milliseconds wont fit
  // in the target type, so this eventually ends up in implementation defined
  // behaviour and it gives the wrong result.
  const Sec from{ INT_MAX / 1000 + 1 }; // change +1 to -1, and it works
  const mSec to = from;
  assert(to.count() / 1000 == from.count() && "oops, wrong answer!");
}

void
example2_undefined_behaviour_inside_chrono()
{
  using Sec = std::chrono::duration<long long>;
  using mSec = std::chrono::duration<long long, std::milli>;

  // this will cause an internal signed overflow inside chrono, which is
  // undefined behaviour. Here is what ubsan says on my machine:
  // /usr/include/c++/7/chrono:176:38: runtime error: signed integer overflow:
  // 9223372036854776 * 1000 cannot be represented in type 'long long int'

  const Sec from{ LLONG_MAX / 1000 + 1 };
  const mSec to = from;
  assert(to.count() / 1000 == from.count() && "oops, wrong answer!");
}

void
example3_result_doesnt_fit()
{
  using Sec = std::chrono::duration<double>;
  using mSec = std::chrono::duration<int, std::milli>;

  // converting a large number of seconds to milliseconds wont fit
  // in the target type, so this eventually ends up in implementation defined
  // behaviour and it gives the wrong result. prove this using a large
  // tolerance.
  using LargeInt = long long;
  const LargeInt expected_ms = LargeInt{ INT_MAX } / 1000 * 1000;
  const Sec from{ expected_ms / 1000 + 1 }; // -1 works fine, +1 overflows
  const mSec to = std::chrono::duration_cast<mSec>(from);
  const auto diff_ms = to.count() - expected_ms;
  std::cout << " from=" << from.count() << "s to=" << to.count()
            << "ms diff=" << diff_ms << "ms\n";
  assert(std::abs(diff_ms) <= 1000 && "oops, wrong answer!");
}

void
example4_overflow_to_inf()
{
  using Sec = std::chrono::duration<float>;
  using mSec = std::chrono::duration<float, std::milli>;

  // we will get inf as a result for this case.
  const double expected_ms = FLT_MAX;
  const Sec from{
    expected_ms / 1000 * (1.0f + std::numeric_limits<float>::epsilon())
  }; // - works fine, + overflows
  const mSec to = std::chrono::duration_cast<mSec>(from);
  const auto diff_ms = to.count() - expected_ms;
  std::cout << " from=" << from.count() << "s to=" << to.count()
            << "ms diff=" << diff_ms << "ms\n";
  assert(std::abs(diff_ms) / expected_ms <= 1e-6 && "oops, wrong answer!");
}

int
main(int argc, char* argv[])
{
  // example1_wrong_result_after_overflow();
  // example2_undefined_behaviour_inside_chrono();
  // example3_result_doesnt_fit();
  example4_overflow_to_inf();
}
