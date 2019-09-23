#ifndef INGESTION_PORTAL_H
#define INGESTION_PORTAL_H

#include "util/tcp-server.hpp"
#include "exec/ingestion-executor.hpp"
#include "admin/definitions.hpp"

const uint32_t NUM_INGESTION_REQ_PACKETS = 5;
typedef union {
    ingestion_t request[NUM_INGESTION_REQ_PACKETS];
    packet_directive_t response;
} ingestion_packet_t;

/*
    The EngineIngestionPortal class is a portal mounted
    on top of a TCP server to read contribution and update
    data and feed the result into the ingestion executor. 
*/
class EngineIngestionPortal : private EnginePortal {
    public:
        EngineIngestionPortal(EngineIngestionExecutor& executor, uint16_t port);
        virtual ~EngineIngestionPortal();
        virtual void run();
        virtual void shutdown();

    private:
        EngineIngestionExecutor& executor;
        TCPServer<ingestion_packet_t> server;
};

#endif