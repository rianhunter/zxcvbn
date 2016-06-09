#include <zxcvbn/frequency_lists.hpp>

#include <zxcvbn/_frequency_lists.hpp>

#include <unordered_map>

namespace zxcvbn {

RankedDicts convert_to_ranked_dicts(std::unordered_map<DictionaryTag, RankedDict> & ranked_dicts) {
  RankedDicts build;

  for (const auto & item : ranked_dicts) {
    build.insert(item);
  }

  return build;
}

RankedDicts default_ranked_dicts() {
  return convert_to_ranked_dicts(_frequency_lists::get_default_ranked_dicts());
}


}
