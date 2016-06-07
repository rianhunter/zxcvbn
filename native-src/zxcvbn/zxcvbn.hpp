#ifndef __ZXCVBN__ZXCVBN_HPP
#define __ZXCVBN__ZXCVBN_HPP

#include <zxcvbn/feedback.hpp>
#include <zxcvbn/scoring.hpp>
#include <zxcvbn/time_estimates.hpp>

#include <string>
#include <vector>

namespace zxcvbn {

struct ZxcvbnResult {
  scoring::ScoringResult scoring;
  time_estimates::AttackTimes attack_times;
  feedback::Feedback feedback;
};

ZxcvbnResult zxcvbn(const std::string & password, const std::vector<std::string> & user_inputs);

}

#endif
