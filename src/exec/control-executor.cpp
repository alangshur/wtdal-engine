#include "exec/control-executor.hpp"
using namespace std;
 
EngineControlExecutor::EngineControlExecutor(EngineContributionStore& 
    contribution_store) : contribution_store(contribution_store) {
    this->logger.log_message("EngineControlExecutor", "Initializing control executor.");
}

EngineControlExecutor::~EngineControlExecutor() {}

void EngineControlExecutor::run() {
    try {
        while (true) {

            // deliver contribution store outliers
            this->contribution_store.wait_for_outlier();
            if (this->shutdown_flag) break;
            outlier_t outlier = this->contribution_store.fetch_outlier();
            unique_lock<mutex> lk(this->outlier_queue_mutex);
            this->outlier_queue.push(outlier); 
            outlier_queue_size++;
        }
    }
    catch(exception& e) {
        if (!this->global_shutdown_in_progress()) {
            this->logger.log_error("EngineControlExecutor", "Fatal error: " 
                + string(e.what()) + ".");
            this->report_fatal_error();
        }
    }

    this->notify_shutdown();
}

void EngineControlExecutor::shutdown() {

    // trigger shutdown
    this->shutdown_flag = true;
    this->contribution_store.trigger_outlier_wait();

    // wait for shutdown
    this->wait_shutdown();
    this->logger.log_message("EngineControlExecutor", "Successfully shutdown "
        "control executor.");
}

outlier_t EngineControlExecutor::fetch_outlier() {
    if (!outlier_queue_size) return { No, 0 };
    unique_lock<mutex> lk(this->outlier_queue_mutex);
    outlier_t outlier =  this->outlier_queue.front();
    this->outlier_queue.pop();
    outlier_queue_size--;
    return outlier;
}