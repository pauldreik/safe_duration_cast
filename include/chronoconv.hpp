#include <stdexcept>

#include <detail/chronoconv_detail.hpp>

namespace safe_duration_cast {
/**
 * converts From to To (potentially lossy, just like std::chrono::duration_cast,
 * but avoids overflow
 */
template<typename To, typename From>
constexpr To
safe_duration_cast(From from, int& ec)
{
  ec = 0;
  static_assert(detail::is_duration(From{}), "From is not a duration");
  static_assert(detail::is_duration(To{}), "To is not a duration");

  // const auto to = std::chrono::duration_cast<To>(from);
  const auto to = detail::duration_cast_impl<To>(from, ec);
  return to;
}

// throwing version
template<typename To, typename From>
To
safe_duration_cast(From from)
{
  int ec = 0;
  auto ret = safe_duration_cast<To, From>(from, ec);
  if (ec) {
    throw std::runtime_error("failed conversion");
  }
  return ret;
} // func
} // namespace safe_duration_cast
