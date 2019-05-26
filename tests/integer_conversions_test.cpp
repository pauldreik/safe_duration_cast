/*
 * By Paul Dreik 2019
 *
 * License:
 * dual license, pick your choice. Either Boost license 1.0, or GPL(v2 or later,
 * at your option).
 */
#include <catch.hpp>

#include "testsupport.hpp"
#include <climits>
#include <cstdint>
#include <detail/lossless_conversion.hpp>
#include <type_traits>

// see
// https://github.com/catchorg/Catch2/blob/master/docs/tostring.md#catchstringmaker-specialisation
namespace Catch {
template<>
struct StringMaker<tests::Int128_t>
{
  static std::string convert(tests::Int128_t const& /*value*/)
  {
    return "can't print tests::Int128_t";
  }
};
}

TEST_CASE("negative value to unsigned")
{
  const int negative = -1;
  int err;
  const auto to =
    safe_duration_cast::lossless_integral_conversion<unsigned>(negative, err);

  REQUIRE(err != 0);
}

TEST_CASE("positive value to unsigned")
{
  const int positive = 123;
  int err;
  const auto to =
    safe_duration_cast::lossless_integral_conversion<unsigned>(positive, err);

  REQUIRE(err == 0);
  REQUIRE(positive == to);
}

/**
 * makes sure from can be converted to To and have the correct value.
 */
template<typename From, typename To>
void
verifyLossless(const From from)
{
  int err = 0;
  const auto to =
    safe_duration_cast::lossless_integral_conversion<To>(from, err);
  REQUIRE(err == 0);

  // use a very large type to make sure the result is as expected
  REQUIRE(tests::Int128_t{ to } == tests::Int128_t{ from });
}
/**
 * makes sure conversion of from to To gives an error
 */
template<typename From, typename To>
void
verifyError(const From from)
{
  int err = 0;
  const auto to =
    safe_duration_cast::lossless_integral_conversion<To>(from, err);
  REQUIRE(err != 0);
}

/**
 * makes sure conversion either signals an error, or is correct.
 */
template<typename From, typename To>
void
verifyErrorOrCorrect(const From from)
{
  int err = 0;
  const auto to =
    safe_duration_cast::lossless_integral_conversion<To>(from, err);
  if (err) {
    REQUIRE(err != 0);
  } else {
    // use a very large type to make sure the result is as expected
    REQUIRE(tests::Int128_t{ to } == tests::Int128_t{ from });
  }
}

/**
 * if value fits within what From can hold, go on and
 * verify conversion of From to To either signals an error, or is correct.
 */
template<typename From, typename To, typename Value>
void
verifyErrorOrCorrectValue(const Value value)
{
  const tests::Int128_t from{ value };
  const tests::Int128_t min{ std::numeric_limits<From>::min() };
  const tests::Int128_t max{ std::numeric_limits<From>::max() };
  if (from < min || from > max) {
    // value does not fit, ignore this test.
    return;
  }
  verifyErrorOrCorrect<From, To>(value);
}

template<typename Int>
void
verifyFitsInUnsigned()
{
  using Unsigned = typename std::make_unsigned<Int>::type;
  using Signed = typename std::make_signed<Int>::type;

  // going from unsigned to signed, the good cases
  verifyLossless<Unsigned, Signed>(0);
  verifyLossless<Unsigned, Signed>(1);
  verifyLossless<Unsigned, Signed>(std::numeric_limits<Signed>::max());

  // going from unsigned to signed, the cases that should error out
  verifyError<Unsigned, Signed>(Unsigned{ std::numeric_limits<Signed>::max() } +
                                1);
  verifyError<Unsigned, Signed>(std::numeric_limits<Unsigned>::max());

  // going from signed to unsigned, the good cases
  verifyLossless<Signed, Unsigned>(0);
  verifyLossless<Signed, Unsigned>(1);
  verifyLossless<Signed, Unsigned>(std::numeric_limits<Signed>::max());

  // going from signed to unsigned, the cases that should error out
  verifyError<Signed, Unsigned>(std::numeric_limits<Signed>::min());
  verifyError<Signed, Unsigned>(std::numeric_limits<Signed>::min() + 1);
  verifyError<Signed, Unsigned>(-1);
}

TEST_CASE("signed/unsigned conversion of the same size")
{
  verifyFitsInUnsigned<char>();
  verifyFitsInUnsigned<short>();
  verifyFitsInUnsigned<int>();
  verifyFitsInUnsigned<long>();
  verifyFitsInUnsigned<long long>();
  verifyFitsInUnsigned<std::intmax_t>();
}

template<typename From, typename To>
void
verifyConversionsDepth5(tests::Int128_t value)
{
  // investigate those interesting numbers just above and below
  verifyErrorOrCorrectValue<From, To>(value - 2);
  verifyErrorOrCorrectValue<From, To>(value - 1);
  verifyErrorOrCorrectValue<From, To>(value);
  verifyErrorOrCorrectValue<From, To>(value + 1);
  verifyErrorOrCorrectValue<From, To>(value + 2);
}

std::vector<tests::Int128_t>
makeInterestingValues()
{
  // pick generally interesting values. copy paste from
  // https://en.cppreference.com/w/cpp/types/climits

  // don't care about the repetitions, we can afford the extra milliseconds it
  // takes to execute for the ease of understanding and not missing anything.
  const tests::Int128_t values[] = { 0,         PTRDIFF_MIN,    PTRDIFF_MAX,
                                     SIZE_MAX,  SIG_ATOMIC_MIN, SIG_ATOMIC_MAX,
                                     WCHAR_MIN, WCHAR_MAX,      WINT_MIN,
                                     WINT_MAX,  CHAR_MIN,

                                     CHAR_MAX,  SCHAR_MIN,      SHRT_MIN,
                                     INT_MIN,   LONG_MIN,       LLONG_MIN,
                                     SCHAR_MAX, SHRT_MAX,       INT_MAX,
                                     LONG_MAX,  LLONG_MAX,      UCHAR_MAX,
                                     USHRT_MAX, UINT_MAX,       ULONG_MAX,
                                     ULLONG_MAX };
  std::vector<tests::Int128_t> retval;

  for (auto elem : values) {
    for (int i = -10; i < 10; ++i) {
      retval.push_back(elem + i);
    }
  }
  std::sort(retval.begin(), retval.end());
  auto newend = std::unique(retval.begin(), retval.end());
  retval.erase(newend, retval.end());
  return retval;
}

template<typename From, typename To>
void
verifyConversionsDepth4()
{
  // verify the most interesting values that can be problematic
  static const auto interesting = makeInterestingValues();
  for (auto value : interesting) {
    verifyErrorOrCorrectValue<From, To>(value);
  }

  // explicitly pick all interesting values of From
  verifyConversionsDepth5<From, To>(std::numeric_limits<From>::min());
  verifyConversionsDepth5<From, To>(std::numeric_limits<From>::max());

  // explicitly pick all interesting values of To
  verifyConversionsDepth5<From, To>(std::numeric_limits<To>::min());
  verifyConversionsDepth5<From, To>(std::numeric_limits<To>::max());
}

template<typename From, typename To>
void
verifyConversionsDepth3()
{
  using UnsignedTo = typename std::make_unsigned<To>::type;
  using SignedTo = typename std::make_signed<To>::type;
  using UnsignedFrom = typename std::make_unsigned<From>::type;
  using SignedFrom = typename std::make_signed<From>::type;

  verifyConversionsDepth4<UnsignedFrom, UnsignedTo>();
  verifyConversionsDepth4<UnsignedFrom, SignedTo>();
  verifyConversionsDepth4<SignedFrom, UnsignedTo>();
  verifyConversionsDepth4<SignedFrom, SignedTo>();
}

template<typename From>
void
verifyConversionsDepth2()
{
  verifyConversionsDepth3<From, char>();
  verifyConversionsDepth3<From, short>();
  verifyConversionsDepth3<From, int>();
  verifyConversionsDepth3<From, long>();
  verifyConversionsDepth3<From, long long>();
  verifyConversionsDepth3<From, std::intmax_t>();
}

TEST_CASE("conversion should error or give the correct result")
{
  verifyConversionsDepth2<char>();
  verifyConversionsDepth2<short>();
  verifyConversionsDepth2<int>();
  verifyConversionsDepth2<long>();
  verifyConversionsDepth2<long long>();
  verifyConversionsDepth2<std::intmax_t>();
}
