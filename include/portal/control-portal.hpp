#pragma once
#ifndef CONTROL_PORTAL_H
#define CONTROL_PORTAL_H

#include "exec/control-executor.hpp"
#include "util/tcp-server.hpp"
#include "admin/definitions.hpp"

enum control_type {
    Shutdown = 1,
    Outlier = 2,
    Alive = 3
};

typedef struct {
    enum control_type type;
    packet_directive_t directive;
} control_request_t;

typedef union {
    outlier_t outlier;
    packet_directive_t directive;
} control_response_t;

typedef union {
    control_request_t request;
    control_response_t response;
} control_packet_t;

/*
    The EngineControlPortal class is a portal mounted
    on top of a TCP server to read control requests
    and react by managing the entire process or handing
    back requested data (e.g. outlier contribution data).  
*/
class EngineControlPortal : private EnginePortal {
    public:
        EngineControlPortal(EngineControlExecutor& executor, uint16_t port);
        virtual ~EngineControlPortal();
        virtual void run();
        virtual void shutdown();

    private:
        EngineControlExecutor& executor;
        TCPServer<control_packet_t> server;
};
 
#endif