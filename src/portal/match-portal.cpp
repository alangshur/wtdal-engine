#include "portal/match-portal.hpp"
using namespace std;

EngineMatchPortal::EngineMatchPortal(EngineMatchExecutor& executor,
    uint16_t port) : executor(executor), server(port) {
    this->logger.log_message("EngineMatchPortal", "Initializing match portal"
        " on port " + to_string(port) + ".");
}

EngineMatchPortal::~EngineMatchPortal() {}

void EngineMatchPortal::run() {
    try {

        // accept server connection
        this->server.accept_connection();
        this->logger.log_message("EngineMatchPortal", 
            "Connected to valid client.");

        // loop read/write
        while (true) {

            // read new request
            if (this->shutdown_flag) break;
            match_packet_t match_req;
            this->server.read_packet(match_req);
            if (match_req.request != ACK) 
                throw runtime_error("Invalid request");

            // fetch matches
            match_packet_t match_res;
            match_res.response = this->executor.fetch_match();
            this->logger.log_message("EngineMatchPortal", 
                "Received match request and fetched match.");

            // write response
            this->server.write_packet(match_res);
        }

        // close server connection
        this->server.close_connection();
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