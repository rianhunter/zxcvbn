#include <zxcvbn/common_js.hpp>
#include <zxcvbn/scoring.hpp>
#include <zxcvbn/matching.hpp>
#include <zxcvbn/time_estimates.hpp>
#include <zxcvbn/feedback.hpp>

#include <chrono>

#include <emscripten/bind.h>

namespace zxcvbn_js {

emscripten::val password_strength(const std::wstring & wpassword,
                                  const emscripten::val & user_inputs) {
  auto password = to_utf8(wpassword);

  // reset the user inputs matcher on a per-request basis to keep things stateless
  std::vector<std::string> sanitized_inputs;
  auto len = from_val<std::size_t>(user_inputs["length"]);
  for (decltype(len) i = 0; i < len; ++i) {
    auto type_ = from_val<std::string>(user_inputs[i].typeof());
    if (type_ == "string" ||
        type_ == "number" ||
        type_ == "boolean") {
      sanitized_inputs.push_back(from_val<std::string>(user_inputs[i].call<emscripten::val>("toString").call<emscripten::val>("toLowerCase")));
    }
  }

  auto start = std::chrono::high_resolution_clock::now();
  auto matches = zxcvbn::omnimatch(password, sanitized_inputs);
  auto scoring_result = zxcvbn::most_guessable_match_sequence(password, matches);
  auto stop = std::chrono::high_resolution_clock::now();

  using FpSeconds = std::chrono::duration<double, std::chrono::milliseconds::period>;

  auto result = emscripten::val::object();

  result.set("password", wpassword);

  // set scoring results
  auto match_sequence = std::vector<zxcvbn::Match>{};
  for (auto & elt : scoring_result.sequence) {
    match_sequence.push_back(std::move(elt.get()));
  }

  result.set("guesses", to_val(scoring_result.guesses));
  result.set("guesses_log10", to_val(std::log10(scoring_result.guesses)));
  auto js_sequence = to_val(match_sequence);
  fix_up_dictionary_tags(js_sequence);
  fix_up_graph_tags(js_sequence);
  result.set("sequence", js_sequence);

  // set calc_time
  result.set("calc_time", to_val(FpSeconds(stop - start).count()));

  // set attack times
  auto attack_times = zxcvbn::estimate_attack_times(scoring_result.guesses);

  auto crack_times_seconds = emscripten::val::object();
#define CT(v) crack_times_seconds.set(#v, to_val(attack_times.crack_times_seconds.v));
  CT(online_throttling_100_per_hour);
  CT(online_no_throttling_10_per_second);
  CT(offline_slow_hashing_1e4_per_second);
  CT(offline_fast_hashing_1e10_per_second);
#undef CT
  result.set("crack_time_seconds", crack_times_seconds);

  auto crack_time_display = emscripten::val::object();
#define CT(v) crack_time_display.set(#v, to_val(attack_times.crack_times_display.v));
  CT(online_throttling_100_per_hour);
  CT(online_no_throttling_10_per_second);
  CT(offline_slow_hashing_1e4_per_second);
  CT(offline_fast_hashing_1e10_per_second);
#undef CT
  result.set("crack_time_display", crack_time_display);

  result.set("score", to_val(attack_times.score));

  // set feedback
  result.set("feedback", to_val(zxcvbn::get_feedback(attack_times.score, match_sequence)));

  return result;
}

emscripten::val password_strength(const std::wstring & wpassword) {
  return password_strength(wpassword, emscripten::val::array());
}

}

EMSCRIPTEN_BINDINGS(zxcvbn) {
  emscripten::function("password_strength",
                       emscripten::select_overload<
                       emscripten::val
                       (const std::wstring & wpassword)>(
                         &zxcvbn_js::password_strength));
  emscripten::function("password_strength",
                       emscripten::select_overload<
                       emscripten::val
                       (const std::wstring &,
                        const emscripten::val &)>(
                          &zxcvbn_js::password_strength));

}
