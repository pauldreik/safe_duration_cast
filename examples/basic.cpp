#include <cassert>
#include <chrono>
#include <iostream>
#include <limits>
#include <stdexcept>
#include <type_traits>

#include <chronoconv.hpp>

template<typename To, typename From>
void
doit(const From& from)
{
  std::cout << "From.count()=" << from.count() << '\t';
  To to = safe_duration_cast::safe_duration_cast<To>(from);
  std::cout << "To.count()=" << to.count() << '\n';
}

int
main(int argc, char* argv[])
{
  using From = std::chrono::duration<int, std::milli>;
  using To = std::chrono::duration<int, std::deci>;


  const auto to = safe_duration_cast::safe_duration_cast<To>(From{ 1 });

  doit<std::chrono::milliseconds>(std::chrono::seconds{ 23 });
  doit<std::chrono::seconds>(std::chrono::milliseconds{ 23 });
}
