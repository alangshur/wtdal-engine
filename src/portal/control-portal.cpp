#include "portal/control-portal.hpp"
using namespace std;

EngineControlPortal::EngineControlPortal(EngineControlExecutor& executor,
    uint16_t port) : executor(executor), server(port) {
    this->logger.log_message("EngineControlPortal", "Initializing control portal.");
}

EngineControlPortal::~EngineControlPortal() {}

void EngineControlPortal::run() {
    try {
        while (true) {

            // read new request
            this->server.accept_connection();
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
                uint32_t outlier_count = 0;
                for (size_t i = 0; i < NUM_CONTROL_RES_PACKETS; i++) {
                    outlier_t outlier = this->executor.fetch_outlier();
                    if (outlier.type != No) outlier_count++;
                    control_res.response.outliers[i] = outlier;
                }   
                this->logger.log_message("EngineControlPortal", "Received "
                "outlier request and fetched " + to_string(outlier_count) 
                + " outlier(s).");
            }

            // write response
            this->server.write_packet(control_res);
            this->server.close_connection();
        }
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

    // wait for shutdown
    this->wait_shutdown();
    this->logger.log_message("EngineControlPortal", "Successfully shutdown "
        "control portal.");
}
