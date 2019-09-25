#include <mutex>
#include "portal/ingestion-portal.hpp"
using namespace std;

EngineIngestionPortal::EngineIngestionPortal(EngineIngestionExecutor& executor,
    uint16_t port) : executor(executor), server(port) {
    this->logger.log_message("EngineIngestionPortal", "Initializing ingestion portal"
        " on port " + to_string(port) + ".");
}

EngineIngestionPortal::~EngineIngestionPortal() {}

void EngineIngestionPortal::run() {
    try {
        while (true) {

            // read new request
            this->server.accept_connection();
            if (this->shutdown_flag) break;
            ingestion_packet_t ingestion_req;
            this->server.read_packet(ingestion_req);

            // enqueue ingestions
            uint32_t ingestion_count = 0;
            for (size_t i = 0; i < NUM_INGESTION_REQ_PACKETS; i++) {
                if (ingestion_req.request[i].type != IEmpty) {
                    this->executor.add_ingestion(ingestion_req.request[i]);
                    ingestion_count++;
                }
            }
            this->logger.log_message("EngineIngestionPortal", "Received "
                "ingestion request and added " + to_string(ingestion_count) 
                + " ingestion(s).");

            // write response
            ingestion_packet_t ingestion_res;
            ingestion_res.response = ACK;
            this->server.write_packet(ingestion_res);
            this->server.close_connection();
        }
    }
    catch(exception& e) {
        this->server.force_close_connection();
        if (!this->global_shutdown_in_progress()) {
            this->logger.log_error("EngineIngestionPortal", "Fatal error: " 
                + string(e.what()) + ".");
            this->report_fatal_error();
        }
    }

    this->notify_shutdown();
}

void EngineIngestionPortal::shutdown() {
    
    // trigger shutdown
    this->shutdown_flag = true;
    this->server.close_acceptor();

    // wait for shutdown
    this->wait_shutdown();
    this->logger.log_message("EngineIngestionPortal", "Successfully shutdown "
        "ingestion portal.");
}