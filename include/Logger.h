#pragma once
#include <string>
#include <thread>
#include <atomic>
#include <fstream>
#include <chrono>
#include <ctime>
#include <iomanip>
#include <sstream>
#include "concurrentqueue.h"

class Logger {
public:
    // Constructor accepts a filename to log to
    Logger(const std::string& logFileName);
    
    // Destructor ensures that all logs are flushed before exit
    ~Logger();

    // Method to enqueue a log message
    void log(const std::string& message);

private:
    // Method that processes the log queue and writes to the log file
    void processLogs();

    // Method to get the current timestamp as a string
    std::string getCurrentTime();

    moodycamel::ConcurrentQueue<std::string> logQueue;  // Thread-safe queue for storing logs
    std::thread loggingThread;                           // Background thread for writing logs
    std::atomic<bool> running;                           // Flag to indicate whether logging is still running
    std::ofstream logFile;                               // Output file stream for logging
};
