#ifndef __ZXCVBN_H
#define __ZXCVBN_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef double zxcvbn_guesses_t;

struct zxcvbn_match_sequence;
typedef struct zxcvbn_match_sequence *zxcvbn_match_sequence_t;

int zxcvbn_password_strength(const char *pass, const char *const *user_inputs,
                             zxcvbn_guesses_t *guesses,
                             zxcvbn_match_sequence_t *mseq
                             );

void zxcvbn_match_sequence_destroy(zxcvbn_match_sequence_t);

#ifdef __cplusplus
}
#endif

#endif
