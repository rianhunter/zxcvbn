#ifndef __ZXCVBN__ADJACENCY_GRAPHS_HPP
#define __ZXCVBN__ADJACENCY_GRAPHS_HPP

#include <array>
#include <string>
#include <unordered_map>
#include <vector>

namespace zxcvbn {

// XXX: this will be auto generated
enum class GraphTag {
  QWERTY,
  DVORAK,
  KEYPAD,
  MAC_KEYPAD,
};

}

namespace std {

template<>
struct hash<zxcvbn::GraphTag> {
  std::size_t operator()(const zxcvbn::GraphTag & v) const {
    return static_cast<std::size_t>(v);
  }
};

}

namespace zxcvbn {

using Graph = std::unordered_map<std::string, std::vector<std::string>>;
using Graphs = std::unordered_map<GraphTag, Graph>;

const Graphs & graphs();

using degree_t = double;

extern const degree_t KEYBOARD_AVERAGE_DEGREE;
extern const degree_t KEYPAD_AVERAGE_DEGREE;

extern const std::size_t KEYBOARD_STARTING_POSITIONS;
extern const std::size_t KEYPAD_STARTING_POSITIONS;

}

#endif
