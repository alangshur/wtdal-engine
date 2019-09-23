#include <cmath>
#include "core/contribution-store.hpp"
using namespace std;

EngineContributionStore::EngineContributionStore(EngineEloStore& elo_store) : 
    elo_store(elo_store) {}

void EngineContributionStore::add_contribution(cid contribution_id) {

    // build initial contribution
    contribution_t contribution;
    contribution.contribution_id = contribution_id;
    contribution.rating = ELO_INITIAL_RATING;
    contribution.position = this->elo_store.add_contribution(contribution_id, 
        ELO_INITIAL_RATING);

    // store new contribution
    this->contribution_count++;
    unique_lock<mutex> store_lk(this->store_mutex);
    (this->store)[contribution_id].store(contribution);
    store_lk.unlock();

    // update filled elo bucket list
    unique_lock<mutex> elo_store_lk(this->elo_bucket_scorer_mutex);
    this->elo_bucket_scorer.add_sample((uint32_t) round(contribution.rating));
}

void EngineContributionStore::update_contribution(cid contribution_id, elo new_rating) {

    // update old contribution
    contribution_t contribution = (this->store)[contribution_id].load();
    contribution.position = this->elo_store.update_contribution(contribution_id, 
        contribution.position, contribution.rating, new_rating);
    elo old_rating = contribution.rating;
    contribution.rating = new_rating;

    // store new contribution
    unique_lock<mutex> store_lk(this->store_mutex);
    (this->store)[contribution_id].store(contribution);
    store_lk.unlock();

    // update filled elo bucket scorer
    unique_lock<mutex> elo_store_lk(this->elo_bucket_scorer_mutex);
    this->elo_bucket_scorer.remove_sample((uint32_t) round(old_rating));
    this->elo_bucket_scorer.add_sample((uint32_t) round(new_rating));
    enum outlier_type outlier_r = this->elo_bucket_scorer.is_outlier(new_rating);
    elo_store_lk.unlock();

    // queue outliers
    if (outlier_r != No) {
        if (outlier_r == Above) {
            unique_lock<mutex> outlier_lk(this->above_outlier_queue_mutex);
            this->above_outlier_queue.push(contribution_id);
            outlier_lk.unlock();
            this->above_outliers_count++;
            this->outlier_queue_sem.post();
        }
        else if (outlier_r == Below) {
            unique_lock<mutex> outlier_lk(this->below_outlier_queue_mutex);
            this->below_outlier_queue.push(contribution_id);
            outlier_lk.unlock();
            this->above_outliers_count++;
            this->outlier_queue_sem.post();
        }
    }
}

void EngineContributionStore::remove_contribution(cid contribution_id) {

    // remove contribution in elo store
    contribution_t contribution = (this->store)[contribution_id].load();
    this->elo_store.remove_contribution(contribution.rating, contribution.position);

    // remove contribution in contribution store
    this->contribution_count--;
    unique_lock<mutex> store_lk(this->store_mutex);
    this->store.erase(contribution_id);
    store_lk.unlock();
 
    // update filled elo bucket list
    unique_lock<mutex> elo_store_lk(this->elo_bucket_scorer_mutex);
    this->elo_bucket_scorer.remove_sample((uint32_t) round(contribution.rating));
}

elo EngineContributionStore::fetch_contribution_elo(cid contribution_id) {
    unique_lock<mutex> lk(this->store_mutex);
    return (this->store)[contribution_id].load().rating;
}

bool EngineContributionStore::verify_contribution(cid contribution_id) {
    unique_lock<mutex> lk(this->store_mutex);
    return (this->store).find(contribution_id) != this->store.end();
}

cid EngineContributionStore::attempt_fetch_match_item() {
    unique_lock<mutex> lk(this->elo_bucket_scorer_mutex);
    uint32_t rand_bucket = this->elo_bucket_scorer.fetch_random_sample();
    return this->elo_store.cycle_front_contribution(rand_bucket);
}

pair<cid, cid> EngineContributionStore::fetch_match_pair() {
    pair<cid, cid> match_pair;

    // find base match item
    do { match_pair.first = this->attempt_fetch_match_item(); }
    while (!match_pair.first);

    // find valid additional match item
    do {
        do { match_pair.second = this->attempt_fetch_match_item(); }
        while (!match_pair.second);
    }
    while (match_pair.second != match_pair.first);
    return match_pair;
}

uint32_t EngineContributionStore::get_contribution_count() {
    return this->contribution_count;
}

cid EngineContributionStore::dump_above_outlier_until() {
    unique_lock<mutex> lk(this->above_outlier_queue_mutex);
    if (this->above_outlier_queue.size()) {
        cid contribution_id = this->above_outlier_queue.front();
        this->above_outlier_queue.pop();
        return contribution_id;
    }
    else return 0;
}

cid EngineContributionStore::dump_below_outlier_until() {
    unique_lock<mutex> lk(this->below_outlier_queue_mutex);
    if (this->below_outlier_queue.size()) {
        cid contribution_id = this->below_outlier_queue.front();
        this->below_outlier_queue.pop();
        return contribution_id;
    }
    else return 0;
}

outlier_t EngineContributionStore::get_above_outlier() {
    this->above_outliers_count--;
    unique_lock<mutex> lk(this->above_outlier_queue_mutex);
    cid contribution_id = this->above_outlier_queue.front();
    this->above_outlier_queue.pop();
    return { Above, contribution_id };
}

outlier_t EngineContributionStore::get_below_outlier() {
    this->below_outliers_count--;
    unique_lock<mutex> lk(this->below_outlier_queue_mutex);
    cid contribution_id = this->below_outlier_queue.front();
    this->below_outlier_queue.pop();
    return { Below, contribution_id };
}

outlier_t EngineContributionStore::fetch_outlier() {
    if (this->above_outliers_count > this->below_outliers_count) 
        return this->get_above_outlier();
    else if (this->below_outliers_count > this->above_outliers_count) 
        return this->get_below_outlier();
    else if (this->above_outliers_count > 0)   
        return this->get_above_outlier();
    else return get_below_outlier();
}

void EngineContributionStore::wait_for_outlier() {
    this->outlier_queue_sem.wait();
}

void EngineContributionStore::trigger_outlier_wait() {
    this->outlier_queue_sem.post();
}