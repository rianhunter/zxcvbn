#include <zxcvbn/common_js.hpp>

#include <zxcvbn/adjacency_graphs.hpp>

#include <emscripten/emscripten.h>
#include <emscripten/bind.h>

namespace zxcvbn_js {

emscripten::val qwerty_graph() {
  return to_val(zxcvbn::graphs().at(zxcvbn::GraphTag::QWERTY));
}

emscripten::val dvorak_graph() {
  return to_val(zxcvbn::graphs().at(zxcvbn::GraphTag::DVORAK));
}

emscripten::val keypad_graph() {
  return to_val(zxcvbn::graphs().at(zxcvbn::GraphTag::KEYPAD));
}

emscripten::val mac_keypad_graph() {
  return to_val(zxcvbn::graphs().at(zxcvbn::GraphTag::MAC_KEYPAD));
}

}

EMSCRIPTEN_BINDINGS(adjacency_graphs) {
  emscripten::function("_qwerty_graph", &zxcvbn_js::qwerty_graph);
  emscripten::function("_dvorak_graph", &zxcvbn_js::dvorak_graph);
  emscripten::function("_keypad_graph", &zxcvbn_js::keypad_graph);
  emscripten::function("_mac_keypad_graph", &zxcvbn_js::mac_keypad_graph);
}

#ifdef __EMSCRIPTEN__

int main() {
  // workaround: these values are only guaranteed to exist after init
  EM_ASM({
      Module["qwerty"] = Module["_qwerty_graph"]();
      Module["dvorak"] = Module["_dvorak_graph"]();
      Module["keypad"] = Module["_keypad_graph"]();
      Module["mac_keypad"] = Module["_mac_keypad_graph"]();
    });
  emscripten_exit_with_live_runtime();
  return 0;
}

#endif
