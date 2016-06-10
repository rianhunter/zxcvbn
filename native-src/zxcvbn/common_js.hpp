#ifndef __ZXCVBN__COMMON_JS_HPP
#define __ZXCVBN__COMMON_JS_HPP

#include <zxcvbn/common.hpp>

#include <emscripten/bind.h>

#include <string>
#include <vector>
#include <unordered_map>

namespace zxcvbn_js {

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
       std::is_same<std::decay_t<T>, std::string>::value ||
       std::is_same<std::decay_t<T>, std::wstring>::value
       >> {
  static T from(const emscripten::val & val) {
    return val.as<T>();
  }
  static emscripten::val to(const T & val) {
    return emscripten::val(val);
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

template<>
struct val_converter<zxcvbn::Match> {
//  static zxcvbn::Match from(const emscripten::val &);

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
      result.set("_dictionary_tag", to_val(std::underlying_type_t<zxcvbn::DictionaryTag>(dmatch.dictionary_tag)));
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
      result.set("_graph", to_val(std::underlying_type_t<zxcvbn::GraphTag>(dmatch.graph)));
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

}

#endif
