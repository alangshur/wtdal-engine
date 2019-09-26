#include "portal/control-portal.hpp"
using namespace std;

EngineControlPortal::EngineControlPortal(EngineControlExecutor& executor,
    uint16_t port) : executor(executor), server(port) {
    this->logger.log_message("EngineControlPortal", "Initializing control portal" 
        " on port " + to_string(port) + ".");
}

EngineControlPortal::~EngineControlPortal() {}

void EngineControlPortal::run() {
    try {

        // accept server connection
        this->server.accept_connection();
        this->logger.log_message("EngineControlPortal", 
            "Connected to valid client.");

        // loop read/write 
        while (true) {

            // read new request
            if (this->shutdown_flag) break;
            control_packet_t control_req;
            this->server.read_packet(control_req);

            control_packet_t control_res;
            if ((control_req.request.type == Shutdown) && 
                (control_req.request.directive == ACK)) {
                
                // build response
                control_res.response.directive = ACK;

                // trigger shutdown
                this->logger.log_message("EngineControlPortal", "Received "
                    "valid shutdown request to begin process shutdown.");
                this->report_fatal_error();
            }
            else if ((control_req.request.type == Alive) && 
                (control_req.request.directive == ACK)) {
                
                // build response
                control_res.response.directive = ACK;
                this->logger.log_message("EngineControlPortal", "Received "
                "and confirmed alive request.");
            } 
            else {
                
                // build response
                control_res.response.outlier = this->executor.fetch_outlier();
                this->logger.log_message("EngineControlPortal", "Received "
                    "outlier request and fetched outlier.");
            }

            // write response
            this->server.write_packet(control_res);
        }

        // close server connection
        this->server.close_connection();
    }
    catch(exception& e) {
        this->server.force_close_connection();
        if (!this->global_shutdown_in_progress()) {
            this->logger.log_error("EngineControlPortal", "Fatal error: " 
                + string(e.what()) + ".");
            this->report_fatal_error();
        }
    }

    this->notify_shutdown();
}

void EngineControlPortal::shutdown() {
    
    // trigger shutdown
    this->shutdown_flag = true;
    this->server.close_acceptor();
    this->server.stop_context();
    this->server.shutdown_socket();

    // wait for shutdown
    this->wait_shutdown();
    this->logger.log_message("EngineControlPortal", "Successfully shutdown "
        "control portal.");
}
