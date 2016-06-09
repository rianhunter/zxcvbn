#ifndef __ZXCVBN__FREQUENCY_LISTS_HPP
#define __ZXCVBN__FREQUENCY_LISTS_HPP

#include <zxcvbn/frequency_lists_common.hpp>
#include <zxcvbn/_frequency_lists.hpp>

#include <unordered_map>

#include <cstdint>

namespace zxcvbn {

using DictionaryTag = _frequency_lists::DictionaryTag;

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

using RankedDicts = std::unordered_map<DictionaryTag, const RankedDict &>;

RankedDicts convert_to_ranked_dicts(std::unordered_map<DictionaryTag, RankedDict> & ranked_dicts);
RankedDicts default_ranked_dicts();

}

#endif
