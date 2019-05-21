# safe_duration_cast
TL;DR
a safe alternative to [std::chrono::duration_cast](https://en.cppreference.com/w/cpp/chrono/duration/duration_cast)
## The problem
Converting between std::chrono::duration types can invoke

 - overflow
 - underflow
 - undefined behaviour

For instance, the following may be an unpleaseant surprise:
```cpp
  using Sec = std::chrono::duration<int>;
  using mSec = std::chrono::duration<int, std::milli>;
  const Sec from{ INT_MAX / 1000 + 1 }; // change +1 to -1, and it works
  const mSec to = from;
  assert(to.count() / 1000 == from.count() && "oops, wrong answer!");
```
The above assert will trigger. There is luckily no ub, since std::chrono::duration_cast internally uses a very big integer type for the intermediate calculations. But if you have a larger type, you can expose undefined behaviour inside chrono. See all the examples at [problematic examples](examples/demo_of_problem_to_be_solved.cpp)

## The solution
This library provides the [function template](include/chronoconv.hpp)
```cpp
namespace safe_duration_cast {
template<typename To, typename From>
constexpr To
safe_duration_cast(From from, int& ec);
}
```
which works like [std::chrono::duration_cast](https://en.cppreference.com/w/cpp/chrono/duration/duration_cast), but with error checking. (limitations apply, see below).

The result will either be correct, or the error code ec will be set to a nonzero value.
## Exceptions
In case you like error reporting through exceptions, you can use the throwing variant
```cpp
namespace safe_duration_cast {
template<typename To, typename From>
constexpr To
safe_duration_cast(From from);
}
```
This form either reports the correct result, or throws an exception.
It should be possible to use the library with exceptions disabled (this has yet not been tested), so this function signature is only enabled if the compiler has exceptions enabled (-fno-exceptions on gcc and clang).
## Converting between integral types
An integral type is one which [std::is_integral](https://en.cppreference.com/w/cpp/types/is_integral) says is integral.
For converting between integral durations you wont get under/overflow or the wrong result without ec being set. You won't get exposed to the internal overflow which may happen in std::chrono::duration_cast, or the result not being representable in the output type.
## Converting between floating point types
An floating point type is one which [std::is_floating_point](https://en.cppreference.com/w/cpp/types/is_floating_point) says is:  float, double, long double.

Converting durations with floating point representation won't give you under/overflow. This can happen for instance if you have a huge number and convert it to a smaller unit, say seconds to microseconds.
Instead, the error code ec will be set to a nonzero value.

|input| output| error code ec|
|-----|-------|--------------|
| NaN |  NaN | 0 |
| +Inf | +Inf | 0 |
| -Inf | -Inf | 0 |
| possible to convert | correct result | 0 |
| not possible to convert | - | nonzero |

One can consider what to do with subnormals. Perhaps it had been wise to also signal errors in case subnormal results appear.

## Converting between integral and floating point
This is not yet supported.

## Converting between non-arithmetic types
If you have a duration with a representation which is not recognized as [std::is_arithmetic](https://en.cppreference.com/w/cpp/types/is_arithmetic), you will get a compile time error.

In the future, it might be worth considering conversion of supporting types that [support numeric_limits](https://www.boost.org/doc/libs/1_70_0/libs/multiprecision/doc/html/boost_multiprecision/tut/limits.html), like [boost multiprecision](https://www.boost.org/doc/libs/1_70_0/libs/multiprecision/doc/html/index.html).

## Performance cost
Not yet measured.

## Testing
There are [unit tests](tests) and [fuzz testing](fuzzing). Actually, fuzz testing was used to smoke out all the corner cases. So far it has only been tested on Ubuntu 18.04 64bit, using gcc and clang.

## Acknowledgements
[Arvid Nordberg](https://github.com/arvidn) suggested the use of [cfenv](https://en.cppreference.com/w/cpp/header/cfenv) to search for floating point exceptions, thanks!

## License
The idea is to have a permissive license. Therefore, it is dual licensed so everyone gets what they want. If this is not good enough, let me know.

 - Boost 1.0
 - GNU GPL v2 (or later, at your option)
