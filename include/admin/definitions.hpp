#pragma once
#ifndef DEFINITIONS_H
#define DEFINITIONS_H

#include <atomic>
#include <stdlib.h>
#include "util/logger.hpp"
#include "util/semaphore.hpp"

// define portal ports
const uint16_t INGESTION_PORT = 9000;
const uint16_t MATCH_PORT = 9001;
const uint16_t CONTROL_PORT = 9002;

// define global types
typedef uint32_t cid;
typedef double elo;
typedef enum {
    ACK = 1,
    NAK = 2
} packet_directive_t;

/*
    The ShutdownThread class is a simple parent
    class used to specify simple static shutdown
    constructs that will be shared by the engine
    orchestrator and all the engine thread classes.
*/
class ShutdownThread {
    public:
        static EffSemaphore global_shutdown_sem;
        static std::atomic<bool> global_shutdown_flag;
};

/*
    The EngineThread class is a simple parent
    class used to provide base utilities to 
    individually threaded classes (such as portal
    and exec classes).
*/
class EngineThread : protected ShutdownThread {        
    public:
        EngineThread();

    protected: 
        void report_fatal_error();
        bool global_shutdown_in_progress();
        void notify_shutdown();
        void wait_shutdown();
        virtual void run() = 0;
        virtual void shutdown() = 0;

        Logger logger;
        EffSemaphore binary_shutdown_sem;
        std::atomic<bool> shutdown_flag;
};

/*
    The EngineExecutor class is a simple wrapper
    class that packages the EngineThread class
    for individual executor classes.
*/
class EngineExecutor : protected EngineThread {
    virtual void run() = 0;
    virtual void shutdown() = 0;
};

/*
    The EnginePortal class is a simple wrapper
    class that packages the EngineThread class
    for individual portal classes.
*/
class EnginePortal : protected EngineThread {
    virtual void run() = 0;
    virtual void shutdown() = 0;
};

#endif