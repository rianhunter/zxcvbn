#ifndef __ZXCVBN__SCORING_HPP
#define __ZXCVBN__SCORING_HPP

#include <zxcvbn/common.hpp>

#include <functional>
#include <memory>
#include <string>
#include <regex>
#include <vector>

namespace zxcvbn {

const auto START_UPPER = std::regex(R"(^[A-Z][^A-Z]+$)");
const auto END_UPPER = std::regex(R"(^[^A-Z]+[A-Z]$)");
const auto ALL_UPPER = std::regex(R"(^[^a-z]+$)");
const auto ALL_LOWER = std::regex(R"(^[^A-Z]+$)");

const auto REFERENCE_YEAR = 2016;

struct ScoringResult {
  std::string password;
  guesses_t guesses;
  guesses_log10_t guesses_log10;
  std::vector<std::unique_ptr<Match>> bruteforce_matches;
  std::vector<std::reference_wrapper<Match>> sequence;
};

ScoringResult most_guessable_match_sequence(const std::string & password,
                                            std::vector<Match> & matches,
                                            bool exclude_additive = false);

}

#endif

