#pragma once
#ifndef CONTROL_EXEC_H
#define CONTROL_EXEC_H

#include <atomic>
#include <queue>
#include <mutex>
#include "core/contribution-store.hpp"
#include "admin/definitions.hpp"

/*
    The EngineControlExecutor class pieces together the entire 
    control pipeline for the engine algorithm. It fetches control 
    data and strategically builds responses fulfilling the requests,
    whether that be identifying outlier contributions or shutting
    down the entire process.
*/
class EngineControlExecutor : private EngineExecutor {
    public:
        EngineControlExecutor(EngineContributionStore& contribution_store);
        virtual ~EngineControlExecutor();
        virtual void run();
        virtual void shutdown();
        outlier_t fetch_outlier();
        
    private:
        EngineContributionStore& contribution_store;
        std::queue<outlier_t> outlier_queue;
        std::mutex outlier_queue_mutex;
        std::atomic<uint32_t> outlier_queue_size;
};
 
#endif