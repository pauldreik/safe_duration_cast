/*
 * By Paul Dreik 2019
 *
 * License:
 * dual license, pick your choice. Either Boost license 1.0, or GPL(v2 or later,
 * at your option).
 */
#include <catch.hpp>

#include <chrono>
#include <safe_duration_cast/chronoconv.hpp>

TEST_CASE("what happens with bool")
{
  using Weird = std::chrono::duration<bool>;
  using Seconds = std::chrono::seconds;

  int ec = 0;
  const Weird five =
    safe_duration_cast::safe_duration_cast<Weird>(Seconds{ 5 }, ec);
  REQUIRE((ec != 0 || static_cast<int>(five.count()) == 5));

  // this compiles, but gives the wrong answer
  const Weird chronofive = std::chrono::duration_cast<Weird>(Seconds{ 5 });
  // REQUIRE(static_cast<int>(chronofive.count())==5);
}
