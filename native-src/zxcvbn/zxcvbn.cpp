#include <zxcvbn/zxcvbn.h>

#include <zxcvbn/util.hpp>
#include <zxcvbn/scoring.hpp>
#include <zxcvbn/matching.hpp>

extern "C" {

struct zxcvbn_match_sequence {
  std::vector<zxcvbn::Match> sequence;
};

int zxcvbn_password_strength(const char *password, const char *const *user_inputs,
                             zxcvbn_guesses_t *guesses,
                             zxcvbn_match_sequence_t *pmseq) {
  try {
    std::vector<std::string> sanitized_inputs;
    if (user_inputs) {
      while (*user_inputs) {
        sanitized_inputs.push_back(zxcvbn::util::ascii_lower(*user_inputs));
        user_inputs++;
      }
    }

    auto matches = zxcvbn::omnimatch(password, sanitized_inputs);
    auto result = zxcvbn::most_guessable_match_sequence(password, matches, false);

    if (guesses) {
      *guesses = result.guesses;
    }

    if (pmseq) {
      std::vector<zxcvbn::Match> sequence;
      std::move(result.sequence.begin(), result.sequence.end(),
                std::back_inserter(sequence));
      *pmseq = new zxcvbn_match_sequence{std::move(sequence)};
    }

    return 0;
  }
  catch (...) {
    return -1;
  }
}

void zxcvbn_match_sequence_destroy(zxcvbn_match_sequence_t mseq) {
  delete mseq;
}

}
