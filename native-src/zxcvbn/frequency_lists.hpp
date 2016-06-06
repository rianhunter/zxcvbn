#ifndef __ZXCVBN__FREQUENCY_LISTS_HPP
#define __ZXCVBN__FREQUENCY_LISTS_HPP

#include <string>
#include <unordered_map>
#include <vector>


namespace zxcvbn {

// XXX: this will be auto generated
enum class DictionaryTag {
  PASSWORDS,
  ENGLISH_WIKIPEDIA,
  SURNAMES,
  MALE_NAMES,
  FEMALE_NAMES,
};

}

namespace std {

template<>
struct hash<zxcvbn::DictionaryTag> {
  std::size_t operator()(const zxcvbn::DictionaryTag & v) const {
    return static_cast<std::size_t>(v);
  }
};

}

namespace zxcvbn {

using rank_t = std::size_t;
using RankedDict = std::unordered_map<std::string, rank_t>;

template<class T = std::initializer_list<const char *>>
RankedDict build_ranked_dict(const T & ordered_list) {
  RankedDict result;
  rank_t idx = 1; // rank starts at 1, not 0
  for (const auto & word : ordered_list) {
    result.insert(std::make_pair(word, idx));
    idx += 1;
  }
  return result;
}

using RankedDicts = std::unordered_map<DictionaryTag, const RankedDict &>;

RankedDicts convert_to_ranked_dicts(std::unordered_map<DictionaryTag, RankedDict> &);
RankedDicts default_ranked_dicts();

}

#endif
