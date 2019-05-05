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

template<typename To, typename From>
void
doit(const From& from)
{
  std::cout << "From.count()=" << from.count() << '\t';
  To to = safe_duration_cast::safe_duration_cast<To>(from);
  std::cout << "To.count()=" << to.count() << '\n';
}

template<typename From, typename To>
void
verify_expected_success(From from, To expected)
{
  int err = 0;
  const auto to = safe_duration_cast::safe_duration_cast<To>(from, err);
  static_assert(std::is_same<decltype(to), const To>::value,
                "programming error");
  REQUIRE(err == 0);
  REQUIRE(expected.count() == to.count());
}
template<typename From, typename To>
void
verify_expected_error(From from, To /*expected*/)
{
  int err = 0;
  const auto to = safe_duration_cast::safe_duration_cast<To>(from, err);
  static_assert(std::is_same<decltype(to), const To>::value,
                "programming error");
  REQUIRE(err != 0);
}
TEST_CASE("same representation different ratio")
{
  using Micro = std::chrono::duration<int, std::micro>;
  using Milli = std::chrono::duration<int, std::milli>;
  using Deci = std::chrono::duration<int, std::deci>;
  verify_expected_success(Milli{ 1000 }, Deci{ 10 });
  verify_expected_success(Milli{ 1000 }, Micro{ 1000000 });
  verify_expected_success(Milli{ -1000 }, Deci{ -10 });
  verify_expected_success(Milli{ -1000 }, Micro{ -1000000 });
}
TEST_CASE("same ratio different representation")
{
  using Small = std::chrono::duration<short, std::milli>;
  using Large = std::chrono::duration<int, std::milli>;
  for (auto value : { -2000, -1000, -1, 0, 1, 1000, 2000 }) {
    verify_expected_success(Small{ 1000 }, Large{ 1000 });
  }
}

TEST_CASE("expected to overflow, int")
{
  using Int = int;
  using Micro = std::chrono::duration<Int, std::micro>;
  using Milli = std::chrono::duration<Int, std::milli>;
  verify_expected_error(Milli{ std::numeric_limits<Int>::max() }, Micro{});
}
TEST_CASE("expected to overflow, long")
{
  using Int = long;
  using Micro = std::chrono::duration<Int, std::micro>;
  using Milli = std::chrono::duration<Int, std::milli>;
  verify_expected_error(Milli{ std::numeric_limits<Int>::max() }, Micro{});
}
TEST_CASE("expected to underflow, int")
{
  using Micro = std::chrono::duration<int, std::micro>;
  using Milli = std::chrono::duration<int, std::milli>;
  verify_expected_error(Milli{ std::numeric_limits<int>::min() }, Micro{});
}
TEST_CASE("expected to underflow, long")
{
  using Micro = std::chrono::duration<long, std::micro>;
  using Milli = std::chrono::duration<long, std::milli>;
  verify_expected_error(Milli{ std::numeric_limits<long>::min() }, Micro{});
}
