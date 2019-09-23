#include <cmath>
#include "util/elo-scorer.hpp"

elo EloScorer::calculate_rating(elo target, elo opponent, bool is_winner) {
    
    // calculate expected outcome
    const float exp = (opponent - target) / ELO_N_SCALE;
    const float expected_outcome = 1 / (1 + pow(ELO_EXP_BASE, exp));

    // calculate new rating
    return target + ELO_K_FACTOR * (is_winner - expected_outcome);
}