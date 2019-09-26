#pragma once
#ifndef INGESTION_PORTAL_H
#define INGESTION_PORTAL_H

#include "util/tcp-server.hpp"
#include "exec/ingestion-executor.hpp"
#include "admin/definitions.hpp"

typedef union {
    ingestion_t request;
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