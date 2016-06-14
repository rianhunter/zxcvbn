#include <zxcvbn/util.hpp>

#include <algorithm>
#include <codecvt>
#include <locale>
#include <string>
#include <utility>

#include <cassert>

namespace zxcvbn {

namespace util {

std::string ascii_lower(const std::string & in) {
  const char A = 0x41, Z = 0x5A;
  const char a = 0x61;
  auto result = in;
  std::transform(result.begin(), result.end(), result.begin(),
                 [&] (char c) {
                   return (c >= A && c <= Z
                           ? c - A + a
                           : c);
                 });
  return result;
}

std::string reverse_string(const std::string & in) {
  std::wstring_convert<std::codecvt_utf8<char32_t>, char32_t> conv;
  auto ret = conv.from_bytes(in);
  std::reverse(ret.begin(), ret.end());
  return conv.to_bytes(ret);
}

const std::codecvt_utf8<char32_t> char32_conv;

bool utf8_valid(std::string::const_iterator start,
                std::string::const_iterator end) {
  while (start != end) {
    std::mbstate_t st;

    const char *from = &*start;
    const char *from_end = &*end;
    const char *from_next;

    char32_t new_char;
    char32_t *to_next;

    auto res = char32_conv.in(st, from, from_end, from_next,
                              &new_char, &new_char + 1, to_next);
    if (!((res == std::codecvt_utf8<char32_t>::result::partial &&
           from_next != from_end) ||
          (res == std::codecvt_utf8<char32_t>::result::ok &&
           from_next == from_end))) {
      return false;
    }
    start += (from_next - from);
  }
  return true;
}

bool utf8_valid(const std::string & str) {
  return utf8_valid(str.begin(), str.end());
}

template<class It>
It _utf8_iter(It start, It end) {
  assert(start != end);
  std::mbstate_t st;
  auto amt = char32_conv.length(st, &*start, &*end, 1);
  return start + amt;
}

std::string::iterator utf8_iter(std::string::iterator start,
                                std::string::iterator end) {
  return _utf8_iter(start, end);
}

std::string::const_iterator utf8_iter(std::string::const_iterator start,
                                      std::string::const_iterator end) {
  return _utf8_iter(start, end);
}

std::string::size_type character_len(const std::string & str,
                                     std::string::size_type start,
                                     std::string::size_type end) {
  assert(utf8_valid(str.begin() + start, str.begin() + end));

  std::string::size_type clen = 0;
  for (auto it = str.begin() + start;
        it != str.begin() + end;
        it = utf8_iter(it, str.begin() + end)) {
    clen += 1;
  }
  return clen;
}

std::string::size_type character_len(const std::string & str) {
  return character_len(str, 0, str.size());
}

template<class It>
std::pair<char32_t, It> _utf8_decode(It it, It end) {
  std::mbstate_t st;
  char32_t new_char;
  char32_t *to_next;

  assert(it != end);

  const char *from = &*it;
  const char *from_end = &*end;
  const char *from_next;
  auto res = char32_conv.in(st, from, from_end, from_next,
                            &new_char, &new_char + 1, to_next);
  assert((res == std::codecvt_utf8<char32_t>::result::partial &&
          from_next != from_end) ||
         (res == std::codecvt_utf8<char32_t>::result::ok &&
          from_next == from_end));
  (void) res;

  return std::make_pair(new_char, it + (from_next - from));
}

std::pair<char32_t, std::string::iterator> utf8_decode(std::string::iterator start,
                                                       std::string::iterator end) {
  return _utf8_decode(start, end);
}

std::pair<char32_t, std::string::const_iterator> utf8_decode(std::string::const_iterator start,
                                                             std::string::const_iterator end) {
  return _utf8_decode(start, end);
}

char32_t utf8_decode(const std::string & start,
                     std::string::size_type & idx) {
  auto ret = _utf8_decode(start.begin() + idx, start.end());
  idx += ret.second - (start.begin() + idx);
  return ret.first;
}

}

}
