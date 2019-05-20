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

## Converting between integral types
For converting between integral durations (int, long etc) you wont get under/overflow.
## Converting between floating point types
For converting between floating point durations (float, double, long double) you wont get under/overflow. This can happen for instance if you have a huge number and convert it to a smaller unit, say seconds to microseconds.
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

## Converting between non-standard types
This falls back to invoking [std::chrono::duration_cast](https://en.cppreference.com/w/cpp/chrono/duration/duration_cast)

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
