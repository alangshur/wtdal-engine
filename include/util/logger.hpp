#pragma once
#ifndef LOGGER_H
#define LOGGER_H

#include <mutex>
#include <string>

/*
    The Logger class is a simple logging utility to print
    formatted date-time messages and errors.
*/
class Logger {
    public:
        Logger();
        Logger(bool production);
        void log_message(const std::string& module, const std::string& message);
        void log_error(const std::string& module, const std::string& error);

    private:
        std::string get_formatted_time();

        static std::mutex logger_lock;
        bool production;
};

#endif