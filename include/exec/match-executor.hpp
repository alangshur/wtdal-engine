#ifndef MATCH_EXECUTOR_H
#define MATCH_EXECUTOR_H

#include <mutex>
#include <queue>
#include <condition_variable>
#include "core/contribution-store.hpp"
#include "util/semaphore.hpp"
#include "admin/definitions.hpp"

const uint32_t MATCH_QUEUE_REFILL_SIZE = 5;
const uint32_t MATCH_QUEUE_REFILL_LIMIT = 15;
const uint32_t MIN_MATCH_CONTRIBUTION_COUNT = 5;

enum match_type {
    Valid = 1,
    MEmpty = 2
};

typedef struct {
    enum match_type type;
    cid contribution_id_a;
    cid contribution_id_b;
} match_t;

/*
    The EngineMatchExecutor class pieces together the entire 
    match pipeline for the engine algorithm. It fetches match 
    data from the core infrastructure and intelligently prepares
    this data in a concurrent queue structure for output by the 
    match portal.
*/
class EngineMatchExecutor : private EngineExecutor {
    public:
        EngineMatchExecutor(EngineContributionStore& contribution_store);
        virtual ~EngineMatchExecutor();
        virtual void run();
        virtual void shutdown();
        match_t fetch_match();

    private:
        std::queue<match_t> match_queue;
        std::mutex match_queue_mutex;
        std::condition_variable refill_cv;
        EngineContributionStore& contribution_store;
};

#endif