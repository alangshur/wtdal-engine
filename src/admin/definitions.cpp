#include <iostream>
#include "admin/definitions.hpp"
using namespace std;

EffSemaphore ShutdownThread::global_shutdown_sem(0);
atomic<bool> ShutdownThread::global_shutdown_flag = false;

EngineThread::EngineThread() : shutdown_flag(false) {}

void EngineThread::report_fatal_error() {
    this->global_shutdown_flag = true;
    this->global_shutdown_sem.post();
}

bool EngineThread::global_shutdown_in_progress() {
    return global_shutdown_flag;
}

void EngineThread::notify_shutdown() {
    this->binary_shutdown_sem.post();
}

void EngineThread::wait_shutdown() {
    this->binary_shutdown_sem.wait();
}