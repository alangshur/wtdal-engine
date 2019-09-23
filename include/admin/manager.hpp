#ifndef MANAGER_H
#define MANAGER_H

#include <mutex>
#include <queue>
#include <utility>
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

const uint32_t WORKER_DIR_DEF_SIZE = 10;

typedef struct {
    uint32_t tier;
    uint32_t contribution_count;
    std::string addr;
} worker_t;

/*
    The EngineWorker class is the central 
    management unit for the manager process. It manages
    the massive amount of communication between all the 
    worker processes to rank contributions and generate 
    new match-ups.

    I/O codes:
        - 0: shutdown
        - 1: match
        - 2: contribution
        - 3: update
        - 4: done

    Input structure:
        - Header:
            - 1 byte: input code 
        - Payload:
            - shutdown: NULL (0 bytes)
            - match: NULL (0 bytes)
            - contribution: cid (4 bytes)
            - update: cid, elo, bool (13 bytes)
            - done: NULL (0 bytes)

    Output structure:
        - Header:
            - 1 byte: output code 
        - Payload:
            - shutdown: NULL (0 bytes)
            - match: cid 1, cid 2 (8 bytes)
            - contribution: NULL (0 bytes)
            - update: NULL (0 bytes)
            - done: cid (4 bytes)
*/
class EngineManager : private ShutdownThread {
    public:
        EngineManager(int argc, const char* argv[]);
        void execute();

    private:
        std::pair<cid, cid> get_match();
        void add_contribution(cid contribution_id);
        void update_contribution(cid contribution_id, elo opponent_rating, 
            bool is_winner);
        void control_controller();
        void match_controller();
        void ingestion_controller();
        void trigger_manager_shutdown();
        void wait_manager_controllers();
        void force_worker_shutdown() noexcept;
        void populate_file_descriptors(const char* argv[]);
        void populate_worker_directory(int argc, const char* argv[]);

        std::mutex match_queue_mutex;
        std::queue<std::pair<cid, cid>> match_queue;
        std::vector<std::vector<worker_t>> worker_tier_directory;
        Logger logger;
        uint32_t read_fd;
        uint32_t write_fd;
        cid win_contribution_id;
};

#endif

