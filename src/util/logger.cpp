#include <iostream>
#include <ctime>
#include <string>
#include <iostream>
#include <fstream>
#include <algorithm>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/ini_parser.hpp>
#include "util/logger.hpp"
using namespace std;

mutex Logger::logger_lock;

Logger::Logger() {
    try {
        boost::property_tree::ptree pt;
        boost::property_tree::ini_parser::read_ini("config.ini", pt);
        istringstream ss(pt.get<std::string>("General.ProductionFlag"));
        ss >> boolalpha >> this->production; 
    }
    catch(...) { this->production = false; }
}

void Logger::log_message(const string& module, const string& message) {
    if (production) return;
    lock_guard<mutex> lg(Logger::logger_lock);
    cout 
        << "[LOG] "
        << module
        << " (" << this->get_formatted_time() << "): "
        << message
        << endl << flush;
}

void Logger::log_error(const string& module, const string& error) {
    if (production) return;
    lock_guard<mutex> lg(Logger::logger_lock);
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
    return date + " UTC";
}