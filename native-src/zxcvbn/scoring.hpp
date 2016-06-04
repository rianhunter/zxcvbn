#ifndef __ZXCVBN__SCORING_HPP
#define __ZXCVBN__SCORING_HPP

#include <regex>

namespace zxcvbn {

const auto START_UPPER = std::regex(R"(^[A-Z][^A-Z]+$)");
const auto ALL_UPPER = std::regex(R"(^[^a-z]+$)");

}

#endif

