#include <zxcvbn/_frequency_lists.hpp>
#include <zxcvbn/common_js.hpp>

#include <emscripten/val.h>
#include <emscripten/emscripten.h>

namespace zxcvbn {

namespace _frequency_lists {

static
std::vector<std::string> js_keys(const emscripten::val & val) {
  return zxcvbn_js::from_val<std::vector<std::string>>(emscripten::val::global("Object").call<emscripten::val>("keys", val));
}

static
std::unordered_map<DictionaryTag, RankedDict> build_static_ranked_dicts() {
  auto result = std::unordered_map<DictionaryTag, RankedDict>();

  auto js_frequency_list = emscripten::val::module_property("_frequency_lists");

  assert(!js_frequency_list.isUndefined());

  for (const auto & key : js_keys(js_frequency_list)) {
    RankedDict toadd;
    auto array = js_frequency_list[key];
    auto len = zxcvbn_js::from_val<std::size_t>(array["length"]);
    for (decltype(len) i = 0; i < len; ++i) {
      toadd.insert(std::make_pair(zxcvbn_js::from_val<std::string>(array[i]), i + 1));
    }

    result.insert(std::make_pair(zxcvbn_js::_default_name_to_dict_tag.at(key),
                                 std::move(toadd)));
  }

  return result;
}

// Init _ranked_dicts in emscripten preMain() because it must
// happen after emscripten::val is initialized
static std::unordered_map<DictionaryTag, RankedDict> _ranked_dicts;

extern "C"
int init_ranked_dicts() {
  _frequency_lists::_ranked_dicts = _frequency_lists::build_static_ranked_dicts();
  return 0;
}

static
int schedule_build() {
  EM_ASM_INT({
      Module["addOnPreMain"](function () {Module["dynCall_i"]($0)});
    }, &init_ranked_dicts);
  return 0;
}

extern const auto _schedule_built = schedule_build();

std::unordered_map<DictionaryTag, RankedDict> & get_default_ranked_dicts() {
  return _ranked_dicts;
}

}

}
