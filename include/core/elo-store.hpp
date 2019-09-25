#pragma once
#ifndef ELO_STORE_H
#define ELO_STORE_H

#include <vector>
#include "util/rating-list.hpp"
#include "admin/definitions.hpp"

const uint32_t ELO_STORE_SIZE = 1000000;

/*
    The EngineEloStore class is a linear vector-based data structure
    that enumerates a range of ELO scores, with each index 
    containing a linked-list of contribution IDs. This construct 
    allows for the ratings of all the platform contributions 
    to be tediously tracked and organized.
*/
class EngineEloStore {
    friend class EngineContributionStore;

    public:
        EngineEloStore();
        ~EngineEloStore();

    private:
        c_node* add_contribution(cid contribution_id, elo init_rating);
        c_node* update_contribution(cid contribution_id, 
            c_node* position, elo old_rating, elo new_rating);
        void remove_contribution(elo rating, c_node* position);
        uint32_t get_rating_list_size(elo rating);
        cid cycle_front_contribution(uint32_t elo_bucket);
            
        std::vector<RatingList> store;
};

#endif