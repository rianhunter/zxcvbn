#ifndef __ZXCVBN__ADJACENCY_GRAPHS_HPP
#define __ZXCVBN__ADJACENCY_GRAPHS_HPP

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

}

#endif
