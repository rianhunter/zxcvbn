#include <zxcvbn/common_js.hpp>
#include <zxcvbn/scoring.hpp>

#include <emscripten/emscripten.h>
#include <emscripten/bind.h>

#include <cstdint>

namespace zxcvbn_js {

static
double nCk(double a, double b) {
  return zxcvbn::nCk(a, b);
}

static
double log2(double a) {
  return std::log2(a);
}

static
double log10(double a) {
  return std::log10(a);
}

static
emscripten::val most_guessable_match_sequence(const std::wstring & wpassword,
                                              emscripten::val matches,
                                              bool exclude_additive) {
  auto password = to_utf8(wpassword);

  // NB: preserving the reference semantics of the JS version requires
  //     some careful plumbing (returning input match references,
  //     propagating mutations)

  auto matches_native = val_converter<std::vector<zxcvbn::Match>>::from(matches);

  // create native_match -> input match mapping
  std::unordered_map<zxcvbn::Match *, emscripten::val> match_to_val;
  for (decltype(matches_native.size()) i = 0; i < matches_native.size(); ++i) {
    match_to_val.insert(std::make_pair(&matches_native[i], matches[i]));
  }

  auto native_result = zxcvbn::most_guessable_match_sequence(password, matches_native, exclude_additive);

  // add guesses information to input matches
  for (decltype(matches_native.size()) i = 0; i < matches_native.size(); ++i) {
    auto val = matches[i];
    val.set("guesses", to_val(matches_native[i].guesses));
    val.set("guesses_log10", to_val(matches_native[i].guesses_log10));
  }

  // convert sequence to reuse input matches
  auto sequence_array = emscripten::val::array();
  for (auto & ref : native_result.sequence) {
    auto it = match_to_val.find(&ref.get());
    auto toadd = (it != match_to_val.end()
                  ? it->second
                  : to_val(ref.get()));
    sequence_array.call<void>("push", toadd);
  }

  auto result = emscripten::val::object();

  result.set("password", to_val(native_result.password));
  result.set("sequence", std::move(sequence_array));
  result.set("guesses", to_val(native_result.guesses));
  result.set("guesses_log10", to_val(native_result.guesses_log10));

  return result;
}

static
emscripten::val most_guessable_match_sequence(const std::wstring & wpassword,
                                              emscripten::val matches) {
  return most_guessable_match_sequence(wpassword, std::move(matches), false);
}

static
zxcvbn::guesses_t estimate_guesses(emscripten::val match,
                                   const std::wstring & wpassword) {
  auto password = to_utf8(wpassword);

  auto native_match = from_val<zxcvbn::Match>(match);
  auto result = zxcvbn::estimate_guesses(native_match, password);

  // propagate guess mutations
  match.set("guesses", to_val(native_match.guesses));
  match.set("guesses_log10", to_val(native_match.guesses_log10));

  return result;
}

static
zxcvbn::guesses_t date_guesses(const emscripten::val & match) {
  return zxcvbn::date_guesses(from_val<zxcvbn::Match>(match));
}

static
zxcvbn::guesses_t repeat_guesses(const emscripten::val & match) {
  return zxcvbn::repeat_guesses(from_val<zxcvbn::Match>(match));
}

static
zxcvbn::guesses_t sequence_guesses(const emscripten::val & match) {
  return zxcvbn::sequence_guesses(from_val<zxcvbn::Match>(match));
}

static
zxcvbn::guesses_t regex_guesses(const emscripten::val & match) {
  return zxcvbn::regex_guesses(from_val<zxcvbn::Match>(match));
}

static
zxcvbn::guesses_t spatial_guesses(const emscripten::val & match) {
  return zxcvbn::spatial_guesses(from_val<zxcvbn::Match>(match));
}

static
zxcvbn::guesses_t dictionary_guesses(const emscripten::val & match) {
  return zxcvbn::dictionary_guesses(from_val<zxcvbn::Match>(match));
}

static
zxcvbn::guesses_t uppercase_variations(const emscripten::val & match) {
  return zxcvbn::uppercase_variations(from_val<zxcvbn::Match>(match));
}

static
zxcvbn::guesses_t l33t_variations(const emscripten::val & match) {
  return zxcvbn::l33t_variations(from_val<zxcvbn::Match>(match));
}

}

EMSCRIPTEN_BINDINGS(scoring) {
  emscripten::function("nCk", &zxcvbn_js::nCk);
  emscripten::function("log2", &zxcvbn_js::log2);
  emscripten::function("log10", &zxcvbn_js::log10);
  emscripten::function("most_guessable_match_sequence",
                       emscripten::select_overload<
                       emscripten::val
                       (const std::wstring & password,
                        emscripten::val matches,
                        bool exclude_additive)>
                       (&zxcvbn_js::most_guessable_match_sequence));
  emscripten::function("most_guessable_match_sequence",
                       emscripten::select_overload<
                       emscripten::val
                       (const std::wstring & password,
                        emscripten::val matches)>
                       (&zxcvbn_js::most_guessable_match_sequence));
  emscripten::function("estimate_guesses", &zxcvbn_js::estimate_guesses);
  emscripten::function("date_guesses", &zxcvbn_js::date_guesses);
  emscripten::function("repeat_guesses", &zxcvbn_js::repeat_guesses);
  emscripten::function("sequence_guesses", &zxcvbn_js::sequence_guesses);
  emscripten::function("regex_guesses", &zxcvbn_js::regex_guesses);
  emscripten::constant("MIN_YEAR_SPACE", zxcvbn::MIN_YEAR_SPACE);
  emscripten::constant("REFERENCE_YEAR", zxcvbn::REFERENCE_YEAR);
  emscripten::function("spatial_guesses", &zxcvbn_js::spatial_guesses);
  emscripten::function("dictionary_guesses", &zxcvbn_js::dictionary_guesses);
  emscripten::function("uppercase_variations", &zxcvbn_js::uppercase_variations);
  emscripten::function("l33t_variations", &zxcvbn_js::l33t_variations);
};

#ifdef __EMSCRIPTEN__

int main() {
  // workaround: emscripten::constant() can only handle integrals or aggregates
  // not doubles, also these are dynamically initialized
  EM_ASM_DOUBLE({
      Module["KEYBOARD_AVERAGE_DEGREE"] = $0;
    }, zxcvbn::KEYBOARD_AVERAGE_DEGREE);
  EM_ASM_INT({
      Module["KEYBOARD_STARTING_POSITIONS"] = $0;
    }, zxcvbn::KEYBOARD_STARTING_POSITIONS);
  emscripten_exit_with_live_runtime();
  return 0;
}

#endif
