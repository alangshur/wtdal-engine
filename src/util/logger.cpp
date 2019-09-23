#include <iostream>
#include <ctime>
#include <string>
#include "util/logger.hpp"
using namespace std;

void Logger::log_message(const string& module, const string& message) {
    cout 
        << "[LOG] "
        << module
        << " (" << this->get_formatted_time() << "): "
        << message
        << endl << flush;
}

void Logger::log_error(const string& module, const string& error) {
    cout 
        << "[ERROR] "
        << module
        << " (" << this->get_formatted_time() << "): "
        << error
        << endl << flush;
}

string Logger::get_formatted_time() {
    time_t now = time(0);
    string date = string(asctime(gmtime(&now)));
    date.pop_back();
    date.append(" UTC");
    return date;
}