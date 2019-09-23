#include "exec/match-executor.hpp"
using namespace std;

EngineMatchExecutor::EngineMatchExecutor(EngineContributionStore& contribution_store) 
    : contribution_store(contribution_store) {
    this->logger.log_message("EngineMatchExecutor", "Initializing match executor.");
}

EngineMatchExecutor::~EngineMatchExecutor() {}

void EngineMatchExecutor::run() {
    try {
        while (true) {  

            // wait for refill
            unique_lock<mutex> lk(this->match_queue_mutex);
            this->refill_cv.wait(lk, [&, this]() { 
                if (this->shutdown_flag) return true;
                else if (this->contribution_store.get_contribution_count() < MIN_MATCH_CONTRIBUTION_COUNT) {
                    this->logger.log_message("EngineMatchExecutor", "Contribution count too small for matches.");
                    return false;
                }
                else return (this->match_queue.size() <= MATCH_QUEUE_REFILL_SIZE);
            });
            if (this->shutdown_flag) break;

            // refill matches
            this->logger.log_message("EngineMatchExecutor", "Beginning to refill match queue.");
            while (this->match_queue.size() < MATCH_QUEUE_REFILL_LIMIT) {
                pair<cid, cid> match_pair = this->contribution_store.fetch_match_pair();
                this->match_queue.push({ Valid, match_pair.first, match_pair.second });
            }   
        }
    }
    catch(exception& e) {
        if (!this->global_shutdown_in_progress()) {
            this->logger.log_error("EngineMatchExecutor", "Fatal error: " + string(e.what()) + ".");
            this->report_fatal_error();
        }
    }

    this->notify_shutdown();
}

void EngineMatchExecutor::shutdown() {
    
    // trigger shutdown
    this->shutdown_flag = true;
    this->refill_cv.notify_all();

    // wait for shutdown
    this->wait_shutdown();
    this->logger.log_message("EngineMatchExecutor", "Successfully shutdown "
        "match executor.");
}


match_t EngineMatchExecutor::fetch_match() {

    // fetch new match
    unique_lock lk(this->match_queue_mutex);
    if (!this->match_queue.size()) return { MEmpty, 0, 0 };
    match_t match = this->match_queue.front();
    this->match_queue.pop();
    if (this->match_queue.size() <= MATCH_QUEUE_REFILL_SIZE) this->refill_cv.notify_one();
    return match;
}
