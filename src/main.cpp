#include "admin/worker.hpp"
#include "admin/manager.hpp"
using namespace std;
#include <iostream>

/*
    argv (for manager):
        1 - "worker"/"manager" (string)
        2 - # of worker processes (uint32_t)
        3 - read file descriptor (uint32_t)
        4 - write file descriptor (uint32_t)
        n - worker {(n // 2) - 1} IP (string)
        n + 1 - worker {((n - 1) // 2) - 1} tier (uint32_t)
*/
int main(int argc, const char* argv[]) {

    // launch worker process
    if (string(argv[1]) == "worker") {
        EngineWorker worker;
        worker.execute();
    }

    // launch manager process
    else if (string(argv[1]) == "manager") {
        EngineManager manager(argc, argv);
        manager.execute();
    }

    else return 0;
}