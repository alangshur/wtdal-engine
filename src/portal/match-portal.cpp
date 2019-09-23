#include "portal/match-portal.hpp"
using namespace std;

EngineMatchPortal::EngineMatchPortal(EngineMatchExecutor& executor,
    uint16_t port) : executor(executor), server(port) {
    this->logger.log_message("EngineMatchPortal", "Initializing match portal.");
}

EngineMatchPortal::~EngineMatchPortal() {}

void EngineMatchPortal::run() {
    try {
        while (true) {

            // read new request
            this->server.accept_connection();
            if (this->shutdown_flag) break;
            match_packet_t match_req;
            this->server.read_packet(match_req);
            if (match_req.request != ACK) 
                throw runtime_error("Invalid request");

            // fetch matches
            match_packet_t match_res;
            uint32_t match_count = 0;
            for (size_t i = 0; i < NUM_MATCH_RES_PACKETS; i++) {
                match_t match = this->executor.fetch_match();
                if (match.type != MEmpty) match_count++;
                match_res.response[i] = match;
            }

            // write response
            this->logger.log_message("EngineMatchPortal", 
                "Received match request and fetched " + 
                to_string(match_count) + " match(es).");
            this->server.write_packet(match_res);
            this->server.close_connection();
        }
    }
    catch(exception& e) {
        this->server.force_close_connection();
        if (!this->global_shutdown_in_progress()) {
            this->logger.log_error("EngineMatchPortal", "Fatal error: " 
                + string(e.what()) + ".");
            this->report_fatal_error();
        }
    }

    this->notify_shutdown();
}

void EngineMatchPortal::shutdown() {
    
    // trigger shutdown
    this->shutdown_flag = true;
    this->server.close_acceptor();

    // wait for shutdown
    this->wait_shutdown();
    this->logger.log_message("EngineMatchPortal", "Successfully shutdown "
        "match portal.");
}