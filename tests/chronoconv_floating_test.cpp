/*
 * By Paul Dreik 2019
 *
 * License:
 * dual license, pick your choice. Either Boost license 1.0, or GPL(v2 or later,
 * at your option).
 */
#include <catch.hpp>

#include "testsupport.hpp"
#include <chronoconv.hpp>
#include <type_traits>

TEST_CASE("floating point sunshine upscale")
{
  using Micro = std::chrono::duration<float, std::micro>;
  using Milli = std::chrono::duration<float, std::milli>;
  int err;
  const auto to =
    safe_duration_cast::safe_duration_cast<Micro>(Milli{ 1 }, err);
  REQUIRE(err == 0);
  REQUIRE(to.count() == 1000);
}

TEST_CASE("floating point sunshine downscale")
{
  using Micro = std::chrono::duration<float, std::micro>;
  using Milli = std::chrono::duration<float, std::milli>;
  int err;
  const auto to =
    safe_duration_cast::safe_duration_cast<Milli>(Micro{ 1000 }, err);
  REQUIRE(err == 0);
  REQUIRE(to.count() == 1);
}

/**
 * verify that nan as input gives an error, or NaN as output
 * @param nan
 */
template<typename Rep>
void
verifyNaN(Rep nan)
{
  using Micro = std::chrono::duration<Rep, std::micro>;
  using Milli = std::chrono::duration<Rep, std::milli>;
  int err;
  const auto to =
    safe_duration_cast::safe_duration_cast<Milli>(Micro{ nan }, err);
  if (err == 0) {
    REQUIRE(std::isnan(to.count()));
  } else {
    REQUIRE(err != 0);
  }
}
TEST_CASE("float nan should give nan or error out")
{
  verifyNaN(std::numeric_limits<float>::quiet_NaN());
}
TEST_CASE("double nan should give nan or error out")
{
  verifyNaN(std::numeric_limits<double>::quiet_NaN());
}
TEST_CASE("long double nan should give nan or error out")
{
  verifyNaN(std::numeric_limits<long double>::quiet_NaN());
}
// FIXME - inf

// FIXME - known to overflow (towards +inf)

// FIXME - known to underflow (towards -inf)
