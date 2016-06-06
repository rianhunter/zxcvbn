#ifndef __ZXCVBN__UTIL_HPP
#define __ZXCVBN__UTIL_HPP

#include <string>
#include <type_traits>

#include <cmath>

namespace zxcvbn {

namespace util {

template<typename T, typename T2>
constexpr auto round_div(T a, T2 b)
  -> std::enable_if_t<std::is_integral<decltype(a / b)>::value, long> {
  return (a + (a / 2)) / b;
}

template<typename T, typename T2>
constexpr auto round_div(T a, T2 b)
  -> std::enable_if_t<!std::is_integral<decltype(a / b)>::value, long> {
  return std::lround(a / b);
}

std::string ascii_lower(const std::string &);
std::string ascii_upper(const std::string &);
std::string reverse_string(const std::string &);

}

}

#endif
