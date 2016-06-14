#include <zxcvbn/matching.hpp>

#include <zxcvbn/common_js.hpp>

#include <emscripten/bind.h>

namespace zxcvbn_js {

static
bool empty(const emscripten::val & val) {
  return emscripten::val::global("Object").call<emscripten::val>("keys", val)["length"].as<std::size_t>() == 0;
}

static
zxcvbn::RankedDict user_input_dictionary;

static
void set_user_input_dictionary(const emscripten::val & ordered_list) {
  auto ret = val_converter<std::vector<std::string>>::from(ordered_list);
  user_input_dictionary = zxcvbn::build_ranked_dict(ret);
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

static
void fix_up_dictionary_tags(emscripten::val & result,
                            const std::unordered_map<zxcvbn::DictionaryTag, std::string> & _tag_to_name) {
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

static
void fix_up_graph_tags(emscripten::val & result,
                       const std::unordered_map<zxcvbn::GraphTag, std::string> & _tag_to_name) {
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

static
emscripten::val _dictionary_match(const std::wstring & wpassword,
                                  const emscripten::val & ranked_dictionaries = emscripten::val::undefined(),
                                  bool reversed = false,
                                  bool l33t = false,
                                  const emscripten::val & l33t_table = emscripten::val::undefined()) {
  auto password = to_utf8(wpassword);
  std::unordered_map<zxcvbn::DictionaryTag, std::string> _tag_to_name;
  std::unordered_map<zxcvbn::DictionaryTag, zxcvbn::RankedDict> _store;
  zxcvbn::RankedDicts dicts;
  if (ranked_dictionaries.isUndefined()) {
    dicts = zxcvbn::default_ranked_dicts();

    dicts.insert(std::make_pair(zxcvbn::DictionaryTag::USER_INPUTS,
                                std::cref(user_input_dictionary)));

    _tag_to_name = _default_dict_tag_to_name;
  }
  else {
    auto ranked_dicts = val_converter<std::unordered_map<std::string, zxcvbn::RankedDict>>::from(ranked_dictionaries);
    DictTagType tag_idx = _default_name_to_dict_tag.size();
    for (auto & item : ranked_dicts) {
      auto tag = static_cast<zxcvbn::DictionaryTag>(tag_idx);
      auto it = _default_name_to_dict_tag.find(item.first);
      if (it != _default_name_to_dict_tag.end()) {
        tag = it->second;
      }
      else {
        tag_idx += 1;
      }
      _tag_to_name.insert(std::make_pair(tag, item.first));
      _store.insert(std::make_pair(tag, std::move(item.second)));
    }

    dicts = zxcvbn::convert_to_ranked_dicts(_store);
  }

  auto ret = [&] {
    if (reversed) {
      return zxcvbn::reverse_dictionary_match(password, dicts);
    }
    else if (l33t) {
      std::vector<std::pair<std::string, std::vector<std::string>>> _store;
      auto & ret2 = [&] () -> const std::vector<std::pair<std::string, std::vector<std::string>>> & {
        if (l33t_table.isUndefined()) {
          return zxcvbn::L33T_TABLE;
        }
        else {
          auto ret = val_converter<std::unordered_map<std::string, std::vector<std::string>>>::from(l33t_table);
          std::move(ret.begin(), ret.end(), std::back_inserter(_store));
          return _store;
        }
      }();
      return zxcvbn::l33t_match(password, dicts, ret2);
    }
    else {
      return zxcvbn::dictionary_match(password, dicts);
    }
  }();

  auto result = to_val(ret);

  fix_up_dictionary_tags(result, _tag_to_name);

  return result;
}

static
emscripten::val dictionary_match(const std::wstring & wpassword,
                                 const emscripten::val & ranked_dictionaries) {
  return _dictionary_match(wpassword, ranked_dictionaries);
}


static
emscripten::val dictionary_match(const std::wstring & wpassword) {
  return _dictionary_match(wpassword);
}

static
emscripten::val reverse_dictionary_match(const std::wstring & wpassword,
                                         const emscripten::val & ranked_dictionaries) {
  return _dictionary_match(wpassword, ranked_dictionaries, true);
}

static
emscripten::val reverse_dictionary_match(const std::wstring & wpassword) {
  return _dictionary_match(wpassword, emscripten::val::undefined(), true);
}

static
emscripten::val relevant_l33t_subtable(const std::wstring & wpassword,
                                       const emscripten::val & table) {
  auto ret = val_converter<std::unordered_map<std::string, std::vector<std::string>>>::from(table);
  std::vector<std::pair<std::string, std::vector<std::string>>> ret2;
  std::move(ret.begin(), ret.end(), std::back_inserter(ret2));
  auto result = zxcvbn::relevant_l33t_subtable(to_utf8(wpassword), ret2);
  return to_val(result);
}

static
emscripten::val enumerate_l33t_subs(const emscripten::val & table) {
  auto ret = val_converter<std::unordered_map<std::string, std::vector<std::string>>>::from(table);
  auto result = zxcvbn::enumerate_l33t_subs(ret);
  return to_val(result);
}

static
emscripten::val l33t_match(const std::wstring & wpassword,
                           const emscripten::val & ranked_dictionaries,
                           const emscripten::val & table) {
  return _dictionary_match(wpassword, ranked_dictionaries, false, true, table);
}

static
emscripten::val l33t_match(const std::wstring & wpassword) {
  return _dictionary_match(wpassword, emscripten::val::undefined(), false, true);
}

static
emscripten::val spatial_match(const std::wstring & wpassword,
                              const emscripten::val & graphs_val) {
  auto password = to_utf8(wpassword);
  zxcvbn::Graphs _new_graph;

  std::unordered_map<zxcvbn::GraphTag, std::string> _tag_to_name;
  auto & new_graph = [&] () -> const zxcvbn::Graphs & {
    if (graphs_val.isUndefined()) {
      _tag_to_name = _default_graph_tag_to_name;
      return zxcvbn::graphs();
    }
    else {
      auto graphs = val_converter<std::unordered_map<std::string, zxcvbn::Graph>>::from(graphs_val);

      GraphTagType tag_idx = _default_name_to_graph_tag.size();
      for (const auto & item : graphs) {
        auto tag = static_cast<zxcvbn::GraphTag>(tag_idx);
        auto it = _default_name_to_graph_tag.find(item.first);
        if (it != _default_name_to_graph_tag.end()) {
          tag = it->second;
        }
        else {
          tag_idx += 1;
        }

        _tag_to_name.insert(std::make_pair(tag, item.first));
        _new_graph.insert(std::make_pair(tag, std::move(item.second)));
      }

      return _new_graph;
    }
  }();

  auto result = to_val(zxcvbn::spatial_match(password, new_graph));

  fix_up_graph_tags(result, _tag_to_name);

  return result;
}

static
emscripten::val spatial_match(const std::wstring & wpassword) {
  return spatial_match(wpassword, emscripten::val::undefined());
}

static
emscripten::val sequence_match(const std::wstring & wpassword) {
  return to_val(zxcvbn::sequence_match(to_utf8(wpassword)));
}

static
emscripten::val repeat_match(const std::wstring & wpassword) {
  return to_val(zxcvbn::repeat_match(to_utf8(wpassword)));
}

static
emscripten::val regex_match(const std::wstring & wpassword) {
  return to_val(zxcvbn::regex_match(to_utf8(wpassword), zxcvbn::REGEXEN));
}

static
emscripten::val date_match(const std::wstring & wpassword) {
  return to_val(zxcvbn::date_match(to_utf8(wpassword)));
}

static
emscripten::val omnimatch(const std::wstring & wpassword) {
  auto result = to_val(zxcvbn::omnimatch(to_utf8(wpassword)));

  fix_up_dictionary_tags(result, _default_dict_tag_to_name);
  fix_up_graph_tags(result, _default_graph_tag_to_name);

  return result;
}

}

EMSCRIPTEN_BINDINGS(matching) {
  emscripten::constant("no_util", true);
  emscripten::function("empty", &zxcvbn_js::empty);
  emscripten::function("set_user_input_dictionary", &zxcvbn_js::set_user_input_dictionary);
  emscripten::function("dictionary_match",
                       emscripten::select_overload<emscripten::val(
                         const std::wstring &,
                         const emscripten::val &)>(&zxcvbn_js::dictionary_match));
  emscripten::function("dictionary_match",
                       emscripten::select_overload<emscripten::val(
                         const std::wstring &)>(&zxcvbn_js::dictionary_match));
  emscripten::function("reverse_dictionary_match",
                       emscripten::select_overload<emscripten::val(
                         const std::wstring &,
                         const emscripten::val &)>(&zxcvbn_js::reverse_dictionary_match));
  emscripten::function("reverse_dictionary_match",
                       emscripten::select_overload<emscripten::val(
                         const std::wstring &)>(&zxcvbn_js::reverse_dictionary_match));
  emscripten::function("relevant_l33t_subtable", &zxcvbn_js::relevant_l33t_subtable);
  emscripten::function("enumerate_l33t_subs", &zxcvbn_js::enumerate_l33t_subs);
  emscripten::function("l33t_match",
                       emscripten::select_overload<emscripten::val(
                         const std::wstring &)>(&zxcvbn_js::l33t_match));
  emscripten::function("l33t_match", emscripten::select_overload<emscripten::val(
                         const std::wstring &,
                         const emscripten::val &,
                         const emscripten::val &
                         )>(&zxcvbn_js::l33t_match));
  emscripten::function("spatial_match",
                       emscripten::select_overload<emscripten::val(const std::wstring &)>(&zxcvbn_js::spatial_match));
  emscripten::function("spatial_match",
                       emscripten::select_overload
                       <emscripten::val(const std::wstring &,
                                        const emscripten::val &)>
                       (&zxcvbn_js::spatial_match));
  emscripten::function("sequence_match", &zxcvbn_js::sequence_match);
  emscripten::function("repeat_match", &zxcvbn_js::repeat_match);
  emscripten::function("regex_match", &zxcvbn_js::regex_match);
  emscripten::function("date_match", &zxcvbn_js::date_match);
  emscripten::function("omnimatch", &zxcvbn_js::omnimatch);
}

