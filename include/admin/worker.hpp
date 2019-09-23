#ifndef WORKER_H
#define WORKER_H

#include <thread>
#include <vector>
#include "util/logger.hpp"
#include "core/elo-store.hpp"
#include "core/contribution-store.hpp"
#include "exec/ingestion-executor.hpp"
#include "exec/match-executor.hpp"
#include "exec/control-executor.hpp"
#include "portal/ingestion-portal.hpp"
#include "portal/match-portal.hpp"
#include "portal/control-portal.hpp"
#include "admin/definitions.hpp"

/*
    The EngineWorker class is the central 
    orchestration unit for a single worker process. It manages
    the creation of all the individual modules, as well
    as assigning them to threads and monitoring these 
    child threads.
*/
class EngineWorker : private ShutdownThread {
    public:
        ~EngineWorker();
        void execute();

    private:
        void launch_process();
        void wait_process_shutdown();
        void shutdown_process();

        void build_core();
        void build_exec();
        void build_portal();      

        void shutdown_portal();
        void shutdown_exec();

        Logger logger;

        // core pointers
        EngineEloStore* elo_store;
        EngineContributionStore* contribution_store;

        // exec pointers
        EngineIngestionExecutor* ingestion_exec;
        EngineMatchExecutor* match_exec;
        EngineControlExecutor* control_exec;

        // portal pointers
        EngineIngestionPortal* ingestion_portal;
        EngineMatchPortal* match_portal;
        EngineControlPortal* control_portal;

        // threads
        std::vector<std::thread> portal_threads;
        std::vector<std::thread> exec_threads; 
};

#endif