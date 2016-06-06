#ifndef __ZXCVBN__SCORING_HPP
#define __ZXCVBN__SCORING_HPP

#include <zxcvbn/common.hpp>

#include <functional>
#include <string>
#include <regex>
#include <vector>

namespace zxcvbn {

const auto START_UPPER = std::regex(R"(^[A-Z][^A-Z]+$)");
const auto ALL_UPPER = std::regex(R"(^[^a-z]+$)");
const auto REFERENCE_YEAR = 2016;

struct ScoringResult {
  std::vector<std::reference_wrapper<Match>> sequence;
  guesses_t guesses;
};

ScoringResult most_guessable_match_sequence(const std::string & password,
                                            std::vector<Match> & matches,
                                            bool exclude_additive = false);

}

#endif

