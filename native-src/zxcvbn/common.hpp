#ifndef __ZXCVBN__COMMON_HPP
#define __ZXCVBN__COMMON_HPP

#include <zxcvbn/zxcvbn.h>
#include <zxcvbn/frequency_lists.hpp>

#include <string>

#include <cassert>

namespace zxcvbn {

using guesses_t = zxcvbn_guesses_t;
using guesses_log10_t = int;
using score_t = unsigned;
using idx_t = std::string::size_type;

// Add new match types here
#define MATCH_RUN() \
  MATCH_FN(Dictionary, DICTIONARY, dictionary) \
  MATCH_FN(Spatial, SPATIAL, spatial) \
  MATCH_FN(Repeat, REPEAT, repeat) \
  MATCH_FN(Sequence, SEQUENCE, sequence) \
  MATCH_FN(Regex, REGEX, regex) \
  MATCH_FN(Date, DATE, date)

enum class RegexTag {
  RECENT_YEAR,
};

#define MATCH_FN(_, e, __) e,
enum class MatchPattern {
  MATCH_RUN()
};
#undef MATCH_FN

struct DictionaryMatch {
  static constexpr auto pattern = MatchPattern::DICTIONARY;

  DictionaryTag dictionary_tag;
  rank_t rank;
  bool l33t;
  bool reversed;
};

struct SpatialMatch {
  static constexpr auto pattern = MatchPattern::SPATIAL;

  unsigned turns;
};

class Match;

struct RepeatMatch {
  static constexpr auto pattern = MatchPattern::REPEAT;

  std::string base_token;
};

struct SequenceMatch {
  static constexpr auto pattern = MatchPattern::SEQUENCE;
};

struct RegexMatch {
  static constexpr auto pattern = MatchPattern::REGEX;

  RegexTag regex_tag;
};

struct DateMatch {
  static constexpr auto pattern = MatchPattern::DATE;
};

// Define new match types here

class Match {
private:
  MatchPattern _pattern;
#define MATCH_FN(title, upper, lower) title##Match _##lower;
  union {
    MATCH_RUN()
  };
#undef MATCH_FN

public:
  idx_t i, j;
  std::string token;
  guesses_t guesses;
  guesses_log10_t guesses_log10;

  MatchPattern get_pattern() const {
    return _pattern;
  }

#define MATCH_FN(title, upper, lower)           \
  title##Match & get_##lower() {                \
    assert(get_pattern() == MatchPattern::upper);        \
    return _##lower;                                    \
  }                                             \
                                                \
  const title##Match & get_##lower() const {    \
    assert(get_pattern() == MatchPattern::upper);        \
    return _##lower;                            \
  }

MATCH_RUN()

#undef MATCH_FN
};

}

#endif
