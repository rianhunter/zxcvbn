#ifndef __ZXCVBN__COMMON_JS_HPP
#define __ZXCVBN__COMMON_JS_HPP

#include <zxcvbn/common.hpp>
#include <zxcvbn/feedback.hpp>

#include <zxcvbn/util.hpp>

#include <emscripten/bind.h>

#include <codecvt>
#include <locale>
#include <string>
#include <vector>
#include <unordered_map>

namespace zxcvbn_js {

template<class T, class M>
M _get_member_type(M T::*);

// this makes me very sad
#define GET_MEMBER_TYPE(f) decltype(_get_member_type(f))

template<class T, typename = void>
struct val_converter;

template<class T>
emscripten::val to_val(const T & val) {
  return val_converter<T>::to(val);
}

template<class T>
T from_val(const emscripten::val & val) {
  return val_converter<T>::from(val);
}

template<class T>
T from_val_with_default(const emscripten::val & val, T def) {
  return ((val.isUndefined() || val.isNull())
          ? std::move(def)
          : val_converter<T>::from(val));
}

template<class T>
struct val_converter<
  T, std::enable_if_t<
       std::is_same<std::decay_t<T>, void>::value ||
       std::is_same<std::decay_t<T>, bool>::value ||
       std::is_same<std::decay_t<T>, char>::value ||
       std::is_same<std::decay_t<T>, signed char>::value ||
       std::is_same<std::decay_t<T>, unsigned char>::value ||
       std::is_same<std::decay_t<T>, short>::value ||
       std::is_same<std::decay_t<T>, unsigned short>::value ||
       std::is_same<std::decay_t<T>, int>::value ||
       std::is_same<std::decay_t<T>, unsigned int>::value ||
       std::is_same<std::decay_t<T>, long>::value ||
       std::is_same<std::decay_t<T>, unsigned long>::value ||
       std::is_same<std::decay_t<T>, float>::value ||
       std::is_same<std::decay_t<T>, double>::value ||
       std::is_same<std::decay_t<T>, std::wstring>::value
       >> {
  static T from(const emscripten::val & val) {
    return val.as<T>();
  }
  static emscripten::val to(const T & val) {
    return emscripten::val(val);
  }
};


inline
std::string to_utf8(const std::wstring & elt) {
  std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>> u8towide;
  return u8towide.to_bytes(elt);
}

inline
std::wstring to_wide(const std::string & elt) {
  std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>> u8towide;
  return u8towide.from_bytes(elt);
}

template<>
struct val_converter<std::string> {
  static std::string from(const emscripten::val & val) {
    return to_utf8(from_val<std::wstring>(val));
  }
  static emscripten::val to(const std::string & val) {
    return to_val(to_wide(val));
  }
};

template<typename T>
struct val_converter<std::vector<T>> {
  static std::vector<T> from(const emscripten::val & v) {
    auto l = v["length"].as<unsigned>();

    std::vector<T> rv;
    for(unsigned i = 0; i < l; ++i) {
      rv.push_back(val_converter<T>::from(v[i]));
    }

    return rv;
  };

  static emscripten::val to(const std::vector<T> & v) {
    auto result = emscripten::val::array();

    std::size_t i = 0;
    for (const auto & elt : v) {
      result.set(i, val_converter<T>::to(elt));
      i += 1;
    }
    return result;
  }
};

template<class T>
struct val_converter<std::unordered_map<std::string, T>> {
  static std::unordered_map<std::string, T> from(const emscripten::val & val) {
    std::unordered_map<std::string, T> result;
    auto keys = emscripten::val::global("Object").call<emscripten::val>("keys", val);
    for (auto & str : val_converter<std::vector<std::string>>::from(keys)) {
      result.insert(std::make_pair(str, val_converter<T>::from(val[str])));
    }
    return result;
  }
  static emscripten::val to(const std::unordered_map<std::string, T> & val) {
    auto result = emscripten::val::object();
    for (const auto & item : val) {
      result.set(item.first, to_val(item.second));
    }
    return result;
  }
};

template<>
struct val_converter<zxcvbn::SequenceTag> {
  static zxcvbn::SequenceTag from(const emscripten::val & val) {
    auto s = val_converter<std::string>::from(val);
    if (s == "upper") return zxcvbn::SequenceTag::UPPER;
    else if (s == "lower") return zxcvbn::SequenceTag::LOWER;
    else if (s == "digits") return zxcvbn::SequenceTag::DIGITS;
    else {
      assert(s == "unicode");
      return zxcvbn::SequenceTag::UNICODE;
    }
  }
  static emscripten::val to(const zxcvbn::SequenceTag & val) {
    std::string s = [&] {
      if (val == zxcvbn::SequenceTag::UPPER) return "upper";
      else if (val == zxcvbn::SequenceTag::LOWER) return "lower";
      else if (val == zxcvbn::SequenceTag::DIGITS) return "digits";
      else {
        assert(val == zxcvbn::SequenceTag::UNICODE);
        return "unicode";
      }
    }();
    return to_val(s);
  }
};

template<>
struct val_converter<zxcvbn::RegexTag> {
  static zxcvbn::RegexTag from(const emscripten::val & val) {
    auto s = val_converter<std::string>::from(val);
    if (s == "recent_year") return zxcvbn::RegexTag::RECENT_YEAR;
    else if (s == "alpha_lower") return zxcvbn::RegexTag::ALPHA_LOWER;
    else {
      assert(s == "alphanumeric");
      return zxcvbn::RegexTag::ALPHANUMERIC;
    }
  }
  static emscripten::val to(const zxcvbn::RegexTag & val) {
    std::string s = [&] {
      if (val == zxcvbn::RegexTag::RECENT_YEAR) return "recent_year";
      else if (val == zxcvbn::RegexTag::ALPHA_LOWER) return "alpha_lower";
      else {
        assert(val == zxcvbn::RegexTag::ALPHANUMERIC);
        return "alphanumeric";
      }
    }();
    return to_val(s);
  }
};

template<>
struct val_converter<zxcvbn::PortableRegexMatch> {
  static zxcvbn::PortableRegexMatch from(const emscripten::val & val) {
    return {
      val_converter<std::vector<std::string>>::from(val),
      from_val_with_default<std::size_t>(val["index"], 0),
    };
  }
  static emscripten::val to(const zxcvbn::PortableRegexMatch & val) {
    auto result = emscripten::val::array();
    std::size_t i = 0;
    for (const auto & elt : val.matches) {
      result.set(i, to_val(elt));
    }
    result.set("index", to_val(val.index));
    return result;
  }
};

template<class K, class V>
std::unordered_map<V, K> invert_dict(const std::unordered_map<K, V> & dict) {
  std::unordered_map<V, K> result;
  for (const auto & item : dict) {
    result.insert(std::make_pair(item.second, item.first));
  }
  return result;
}

const auto _default_dict_tag_to_name = std::unordered_map<zxcvbn::DictionaryTag, std::string>{
  {zxcvbn::DictionaryTag::PASSWORDS, "passwords"},
  {zxcvbn::DictionaryTag::ENGLISH_WIKIPEDIA, "english_wikipedia"},
  {zxcvbn::DictionaryTag::FEMALE_NAMES, "female_names"},
  {zxcvbn::DictionaryTag::SURNAMES, "surnames"},
  {zxcvbn::DictionaryTag::US_TV_AND_FILM, "us_tv_and_film"},
  {zxcvbn::DictionaryTag::MALE_NAMES, "male_names"},
  {zxcvbn::DictionaryTag::USER_INPUTS, "user_inputs"},
};

const auto _default_name_to_dict_tag = invert_dict(_default_dict_tag_to_name);

const auto _default_graph_tag_to_name = std::unordered_map<zxcvbn::GraphTag, std::string> {
  {zxcvbn::GraphTag::QWERTY, "qwerty"},
  {zxcvbn::GraphTag::DVORAK, "dvorak"},
  {zxcvbn::GraphTag::KEYPAD, "keypad"},
  {zxcvbn::GraphTag::MAC_KEYPAD, "mac_keypad"},
};

const auto _default_name_to_graph_tag = invert_dict(_default_graph_tag_to_name);

using DictTagType = std::underlying_type_t<zxcvbn::DictionaryTag>;
using GraphTagType = std::underlying_type_t<zxcvbn::GraphTag>;

template<>
struct val_converter<zxcvbn::Match> {
  static zxcvbn::Match from(const emscripten::val & val) {
    auto i = from_val_with_default<zxcvbn::idx_t>(val["i"], 0);
    auto j = from_val_with_default<zxcvbn::idx_t>(val["j"], 0);
    auto token = from_val_with_default<std::string>(val["token"], std::string(j - i + 1, '_'));
    auto tlen = zxcvbn::util::character_len(token);
    if (tlen != (j - i + 1)) {
      j = i + tlen - 1;
    }

#define MATCH_FN(title, upper, lower) {#lower, zxcvbn::MatchPattern::upper},
    const auto _default_name_to_pattern = std::unordered_map<std::string, zxcvbn::MatchPattern>{MATCH_RUN()};
#undef MATCH_FN

#define PARSE_MEMBER(class_, member) \
    from_val<GET_MEMBER_TYPE(&zxcvbn::class_::member)>(val[#member])
#define PARSE_MEMBER_DEF(class_, member, def)                           \
    from_val_with_default<GET_MEMBER_TYPE(&zxcvbn::class_::member)>(val[#member], def)

    auto match = [&] {
      auto pattern = [&] {
        if (val["pattern"].isUndefined()) {
          // NB: object is not tagged with pattern, try inferring from members
          if (!val["base_token"].isUndefined()) {
            return zxcvbn::MatchPattern::REPEAT;
          }
          else if (!val["ascending"].isUndefined()) {
            return zxcvbn::MatchPattern::SEQUENCE;
          }
          else if (!val["regex_name"].isUndefined()) {
            return zxcvbn::MatchPattern::REGEX;
          }
          else if (!val["year"].isUndefined()) {
            return zxcvbn::MatchPattern::DATE;
          }
          else if (!val["graph"].isUndefined()) {
            return zxcvbn::MatchPattern::SPATIAL;
          }
          else if (!val["rank"].isUndefined() ||
                   !val["l33t"].isUndefined()) {
            return zxcvbn::MatchPattern::DICTIONARY;
          }
          return zxcvbn::MatchPattern::UNKNOWN;
        }
        auto pattern_str = val_converter<std::string>::from(val["pattern"]);
        auto it = _default_name_to_pattern.find(pattern_str);
        if (it == _default_name_to_pattern.end()) throw std::runtime_error("invalid match");
        return it->second;
      }();

      switch (pattern) {
      case zxcvbn::MatchPattern::DICTIONARY: {
        auto dictionary_tag = [&] {
          if (val["dictionary_name"].isUndefined()) {
            return static_cast<zxcvbn::DictionaryTag>(0);
          }
          auto dictionary_name = val_converter<std::string>::from(val["dictionary_name"]);

          auto it2 = _default_name_to_dict_tag.find(dictionary_name);
          if (it2 == _default_name_to_dict_tag.end()) throw std::runtime_error("invalid dictionary");
          return it2->second;
        }();

        auto default_sub = std::unordered_map<std::string, std::string>{};

        return zxcvbn::Match(i, j, token, zxcvbn::DictionaryMatch{
            dictionary_tag,
              PARSE_MEMBER_DEF(DictionaryMatch, matched_word, ""),
              PARSE_MEMBER_DEF(DictionaryMatch, rank, 0),
              PARSE_MEMBER_DEF(DictionaryMatch, l33t, false),
              PARSE_MEMBER_DEF(DictionaryMatch, reversed, false),
              PARSE_MEMBER_DEF(DictionaryMatch, sub, default_sub),
              PARSE_MEMBER_DEF(DictionaryMatch, sub_display, ""),
              });
      }
      case zxcvbn::MatchPattern::SPATIAL: {
        auto graph_name = val_converter<std::string>::from(val["graph"]);
        auto it2 = _default_name_to_graph_tag.find(graph_name);
        if (it2 == _default_name_to_graph_tag.end()) throw std::runtime_error("bad graph tag!");
        auto graph = it2->second;

        return zxcvbn::Match(i, j, token, zxcvbn::SpatialMatch{
            graph, PARSE_MEMBER(SpatialMatch, turns),
            PARSE_MEMBER(SpatialMatch, shifted_count)
              });
      }
      case zxcvbn::MatchPattern::SEQUENCE: {
        auto sequence_tag = from_val_with_default<GET_MEMBER_TYPE(&zxcvbn::SequenceMatch::sequence_tag)>(val["sequence_name"], zxcvbn::SequenceTag::UNICODE);
        return zxcvbn::Match(i, j, token, zxcvbn::SequenceMatch{
            sequence_tag,
              PARSE_MEMBER_DEF(SequenceMatch, sequence_space, 0),
              PARSE_MEMBER(SequenceMatch, ascending),
          });
      }
      case zxcvbn::MatchPattern::REPEAT: {
        return zxcvbn::Match(i, j, token, zxcvbn::RepeatMatch{
            PARSE_MEMBER(RepeatMatch, base_token),
            PARSE_MEMBER(RepeatMatch, base_guesses),
            PARSE_MEMBER_DEF(RepeatMatch, base_matches, std::vector<zxcvbn::Match>{}),
            PARSE_MEMBER(RepeatMatch, repeat_count),
              });
      }
      case zxcvbn::MatchPattern::REGEX: {
        auto regex_tag = val_converter<GET_MEMBER_TYPE(&zxcvbn::RegexMatch::regex_tag)>::from(val["regex_name"]);
        return zxcvbn::Match(i, j, token, zxcvbn::RegexMatch{
            regex_tag, PARSE_MEMBER(RegexMatch, regex_match),
          });
      }
      case zxcvbn::MatchPattern::DATE: {
        auto separator = from_val_with_default<std::string>(val["separator"], "");
        return zxcvbn::Match(i, j, token, zxcvbn::DateMatch{
            separator,
            PARSE_MEMBER(DateMatch, year),
            PARSE_MEMBER(DateMatch, month),
            PARSE_MEMBER(DateMatch, day),
            PARSE_MEMBER_DEF(DateMatch, has_full_year, 0),
          });
      }
      case zxcvbn::MatchPattern::BRUTEFORCE: {
        return zxcvbn::Match(i, j, token, zxcvbn::BruteforceMatch{});
      }
      default:
        assert(pattern == zxcvbn::MatchPattern::UNKNOWN);
        return zxcvbn::Match(i, j, token, zxcvbn::UnknownMatch{});
      }
    }();

    match.guesses = from_val_with_default<zxcvbn::guesses_t>(val["guesses"], 0);
    match.guesses_log10 = from_val_with_default<zxcvbn::guesses_log10_t>(val["guesses_log10"], 0);
    return match;
  }

  static emscripten::val to(const zxcvbn::Match & val) {
    auto result = emscripten::val::object();
    result.set("i", to_val(val.i));
    result.set("j", to_val(val.j));
    if (val.token.size()) {
      result.set("token", to_val(val.token));
    }
    if (val.guesses) {
      result.set("guesses", to_val(val.guesses));
      result.set("guesses_log10", to_val(std::log10(val.guesses)));
    }
    switch (val.get_pattern()) {
    case zxcvbn::MatchPattern::DICTIONARY: {
      auto & dmatch = val.get_dictionary();
      result.set("pattern", "dictionary");
      result.set("_dictionary_tag", to_val(DictTagType(dmatch.dictionary_tag)));
      result.set("matched_word", to_val(dmatch.matched_word));
      result.set("rank", to_val(dmatch.rank));
      result.set("l33t", to_val(dmatch.l33t));
      result.set("reversed", to_val(dmatch.reversed));
      result.set("sub", to_val(dmatch.sub));
      result.set("sub_display", to_val(dmatch.sub_display));
      break;
    }
    case zxcvbn::MatchPattern::SPATIAL: {
      auto & dmatch = val.get_spatial();
      result.set("pattern", "spatial");
      result.set("_graph", to_val(GraphTagType(dmatch.graph)));
      result.set("turns", to_val(dmatch.turns));
      result.set("shifted_count", to_val(dmatch.shifted_count));
      break;
    }
    case zxcvbn::MatchPattern::SEQUENCE: {
      auto & dmatch = val.get_sequence();
      result.set("pattern", "sequence");
      result.set("sequence_name", to_val(dmatch.sequence_tag));
      result.set("sequence_space", to_val(dmatch.sequence_space));
      result.set("ascending", to_val(dmatch.ascending));
      break;
    }
    case zxcvbn::MatchPattern::REPEAT: {
      auto & dmatch = val.get_repeat();
      result.set("pattern", "repeat");
      result.set("base_token", to_val(dmatch.base_token));
      result.set("base_guesses", to_val(dmatch.base_guesses));
      result.set("base_matches", to_val(dmatch.base_matches));
      result.set("repeat_count", to_val(dmatch.repeat_count));
      break;
    }
    case zxcvbn::MatchPattern::REGEX: {
      auto & dmatch = val.get_regex();
      result.set("pattern", "regex");
      result.set("regex_name", to_val(dmatch.regex_tag));
      result.set("regex_match", to_val(dmatch.regex_match));
      break;
    }
    case zxcvbn::MatchPattern::DATE: {
      auto & dmatch = val.get_date();
      result.set("pattern", "date");
      result.set("separator", to_val(dmatch.separator));
      result.set("year", to_val(dmatch.year));
      result.set("month", to_val(dmatch.month));
      result.set("day", to_val(dmatch.day));
      result.set("has_full_year", to_val(dmatch.has_full_year));
      break;
    }
    case zxcvbn::MatchPattern::BRUTEFORCE: {
      result.set("pattern", "bruteforce");
      break;
    }
    case zxcvbn::MatchPattern::UNKNOWN: {
      break;
    }
#ifndef NDEBUG
    default:
      assert(false);
#endif
    }
    return result;
  }
};

template<class T>
struct val_converter<zxcvbn::optional::optional<T>> {
  static zxcvbn::optional::optional<T> from(const emscripten::val & val) {
    if (val.isNull()) {
      return zxcvbn::optional::nullopt;
    }
    else {
      return zxcvbn::optional::make_optional(val_converter<T>::from(val));
    }
  }
  static emscripten::val to(const zxcvbn::optional::optional<T> & val) {
    if (!val) {
      return emscripten::val::null();
    }
    else {
      return to_val(*val);
    }
  }
};

template<>
struct val_converter<zxcvbn::Feedback> {
  static emscripten::val to(const zxcvbn::Feedback & val) {
    auto result = emscripten::val::object();
    result.set("warning", to_val(val.warning));
    result.set("suggestions", to_val(val.suggestions));
    return result;
  }
};

inline
void fix_up_dictionary_tags(emscripten::val & result,
                            const std::unordered_map<zxcvbn::DictionaryTag, std::string> & _tag_to_name = _default_dict_tag_to_name) {
  auto len = result["length"].as<std::size_t>();
  for (decltype(len) i = 0; i < len; ++i) {
    auto v = result[i];
    if (v["_dictionary_tag"].isUndefined()) continue;
    auto val = v["_dictionary_tag"].as<DictTagType>();
    auto it = _tag_to_name.find(static_cast<zxcvbn::DictionaryTag>(val));
    assert(it != _tag_to_name.end());
    v.set("dictionary_name", it->second);
  }
}

inline
void fix_up_graph_tags(emscripten::val & result,
                       const std::unordered_map<zxcvbn::GraphTag, std::string> & _tag_to_name = _default_graph_tag_to_name) {
  auto len = result["length"].as<std::size_t>();
  for (decltype(len) i = 0; i < len; ++i) {
    auto v = result[i];
    if (v["_graph"].isUndefined()) continue;
    auto val = v["_graph"].as<GraphTagType>();
    auto it = _tag_to_name.find(static_cast<zxcvbn::GraphTag>(val));
    assert(it != _tag_to_name.end());
    v.set("graph", it->second);
  }
}

}

#endif
