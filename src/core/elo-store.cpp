#include "core/elo-store.hpp"

#include <cmath>
using namespace std;

EngineEloStore::EngineEloStore() : store(ELO_STORE_SIZE) {}

EngineEloStore::~EngineEloStore() {
    for (size_t i = 0; i < store.size(); i++) {
        (this->store)[i].free_list_memory();
    }
}

c_node* EngineEloStore::add_contribution(cid contribution_id, elo init_rating) {
    return (this->store)[(int) round(init_rating)].add_contribution(contribution_id);
}

c_node* EngineEloStore::update_contribution(cid contribution_id, c_node* position, 
    elo old_rating, elo new_rating) {

    // remove old contribution
    (this->store)[(int) round(old_rating)]
        .remove_contribution(position);

    // update contribution
    return (this->store)[(int) round(new_rating)]
        .add_contribution(contribution_id);
}

void EngineEloStore::remove_contribution(elo rating, c_node* position) {
    (this->store)[(int) round(rating)].remove_contribution(position);
}

uint32_t EngineEloStore::get_rating_list_size(elo rating) {
    return (this->store)[(int) round(rating)].size();
}

cid EngineEloStore::cycle_front_contribution(uint32_t elo_bucket) {
    return (this->store)[elo_bucket].cycle_front_contribution();
}
