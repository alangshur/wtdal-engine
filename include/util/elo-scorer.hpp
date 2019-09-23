#ifndef ELO_SCORER_H
#define ELO_SCORER_H

#include "admin/definitions.hpp"

const float ELO_K_FACTOR = 40.0;
const float ELO_N_SCALE = 400.0;
const float ELO_EXP_BASE = 10.0;

/*
    The EloScorer class contains a single method used
    used to quickly calculate updated ELO scores given a result 
    involving two previous ELO scores.
*/
class EloScorer {
    public:
        elo calculate_rating(elo target, elo opponent, bool is_winner);
};

#endif