#include "exec/ingestion-executor.hpp"
using namespace std;

EngineIngestionExecutor::EngineIngestionExecutor(EngineContributionStore& 
    contribution_store) : contribution_store(contribution_store) {
    this->logger.log_message("EngineIngestionExecutor", "Initializing ingestion executor.");
}

EngineIngestionExecutor::~EngineIngestionExecutor() {}

void EngineIngestionExecutor::run() {    
    try {
        while (true) {

            // fetch ingestion
            this->ingestion_queue_sem.wait();
            if (this->shutdown_flag) break;
            unique_lock<mutex> lk(this->ingestion_queue_mutex);
            ingestion_t ingestion = this->ingestion_queue.front();
            this->ingestion_queue.pop();
            lk.unlock();

            // handle contribution ingestion
            if (ingestion.type == Contribution) {
                ingestion_contribution_t& contribution = ingestion.data.contribution;
                this->handle_contribution(contribution);
                this->logger.log_message("EngineIngestionExecutor", "Successfully added "
                    "contribution with ID " + to_string(contribution.contribution_id) + ".");
            }

            // handle update ingestion
            else if (ingestion.type == Update) {
                ingestion_update_t& update = ingestion.data.update;
                elo updated_rating = this->handle_update(update);
                this->logger.log_message("EngineIngestionExecutor", "Successfully updated "
                    "contribution with ID " + to_string(update.contribution_id) + " to rating " 
                    + to_string(updated_rating) + ".");
            }

            // handle remove ingestion
            else if (ingestion.type == Remove) {
                ingestion_remove_t& remove = ingestion.data.remove;
                this->handle_remove(remove);
                this->logger.log_message("EngineIngestionExecutor", "Successfully removed "
                    "contribution with ID " + to_string(remove.contribution_id) + ".");
            }
        }
    }
    catch(exception& e) {
        if (!this->global_shutdown_in_progress()) {
            this->logger.log_error("EngineIngestionExecutor", "Fatal error: " 
                + string(e.what()) + ".");
            this->report_fatal_error();
        }
    }

    this->notify_shutdown();
}

void EngineIngestionExecutor::shutdown() {

    // trigger shutdown
    this->shutdown_flag = true;
    this->ingestion_queue_sem.post();

    // wait for shutdown
    this->wait_shutdown();
    this->logger.log_message("EngineIngestionExecutor", "Successfully shutdown "
        "ingestion executor.");
}

void EngineIngestionExecutor::add_ingestion(ingestion_t& ingestion) {
    unique_lock<mutex> lk(this->ingestion_queue_mutex);
    this->ingestion_queue.push(ingestion);
    this->ingestion_queue_sem.post();
}

void EngineIngestionExecutor::handle_contribution(ingestion_contribution_t& contribution) {
    this->contribution_store.add_contribution(contribution.contribution_id);
}

elo EngineIngestionExecutor::handle_update(ingestion_update_t& update) {

    // calculate new ELO rating
    elo updated_rating = this->scorer.calculate_rating(
        this->contribution_store.fetch_contribution_elo(update.contribution_id),
        update.opponent_rating,
        update.is_winner
    );

    // update contribution
    this->contribution_store.update_contribution(update.contribution_id, updated_rating);
    return updated_rating;
}

void EngineIngestionExecutor::handle_remove(ingestion_remove_t& remove) {
    this->contribution_store.remove_contribution(remove.contribution_id);
}