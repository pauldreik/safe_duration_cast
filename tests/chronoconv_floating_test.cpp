/*
 * By Paul Dreik 2019
 *
 * License:
 * dual license, pick your choice. Either Boost license 1.0, or GPL(v2 or later,
 * at your option).
 */
#include <catch.hpp>

#include "testsupport.hpp"
#include <chrono>
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

/*
 * verify that +inf should give +inf as outut
 */
template<typename Rep>
void
verifyInf(Rep inf)
{
  using Micro = std::chrono::duration<Rep, std::micro>;
  using Milli = std::chrono::duration<Rep, std::milli>;
  int err;
  const auto to =
    safe_duration_cast::safe_duration_cast<Milli>(Micro{ inf }, err);
  REQUIRE(err == 0);
  static_assert(std::numeric_limits<Rep>::has_infinity, "");
  REQUIRE(std::isinf(to.count()));
}
TEST_CASE("float inf should give inf")
{
  verifyInf(+std::numeric_limits<float>::infinity());
  verifyInf(-std::numeric_limits<float>::infinity());
}
TEST_CASE("double inf should give inf")
{
  verifyInf(+std::numeric_limits<double>::infinity());
  verifyInf(-std::numeric_limits<double>::infinity());
}
TEST_CASE("long double inf should give inf")
{
  verifyInf(+std::numeric_limits<long double>::infinity());
  verifyInf(-std::numeric_limits<long double>::infinity());
}

/*
 *  trigger internal overflow, both in positive and negative direction
 */
template<typename Rep>
void
verifyInternalOverflow(Rep large)
{
  using Micro = std::chrono::duration<Rep, std::micro>;
  using Milli = std::chrono::duration<Rep, std::milli>;
  int err;
  const auto to =
    safe_duration_cast::safe_duration_cast<Micro>(Milli{ large }, err);
  REQUIRE(err != 0);
}
TEST_CASE("float overflow/underflow should be noticed")
{
  verifyInternalOverflow(std::numeric_limits<float>::max() / 2);
  verifyInternalOverflow(std::numeric_limits<float>::lowest() / 2);
}
TEST_CASE("double overflow/underflow should be noticed")
{
  verifyInternalOverflow(std::numeric_limits<double>::max() / 2);
  verifyInternalOverflow(std::numeric_limits<double>::lowest() / 2);
}
TEST_CASE("long double overflow/underflow should be noticed")
{
  verifyInternalOverflow(std::numeric_limits<long double>::max() / 2);
  verifyInternalOverflow(std::numeric_limits<long double>::lowest() / 2);
}



/*
 *  make sure the largest finite values are preserved
 */
template<typename Rep>
void
verifyIdentity(Rep val)
{
  using D = std::chrono::duration<Rep>;
  int err;
  const auto to =
    safe_duration_cast::safe_duration_cast<D>(D{ val }, err);
  REQUIRE(err == 0);
  REQUIRE(to.count()==val);
}
TEST_CASE("float limits should be preserved")
{
  verifyIdentity(std::numeric_limits<float>::max());
  verifyIdentity(std::numeric_limits<float>::lowest());
}
TEST_CASE("double limits should be preserved")
{
  verifyIdentity(std::numeric_limits<double>::max());
  verifyIdentity(std::numeric_limits<double>::lowest());
}
TEST_CASE("long doulbe limits should be preserved")
{
  verifyIdentity(std::numeric_limits<long double>::max());
  verifyIdentity(std::numeric_limits<long double>::lowest());
}
