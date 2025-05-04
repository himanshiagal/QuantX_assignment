#include "Logger.h"

Logger::Logger(const std::string& filename) : running(true) {
    logFile.open(filename, std::ios::out | std::ios::app);
    if (!logFile.is_open()) {
        throw std::runtime_error("Failed to open log file.");
    }
    loggingThread = std::thread(&Logger::processLogs, this);
}

Logger::~Logger() {
    running = false;
    if (loggingThread.joinable()) {
        loggingThread.join();
    }
    logFile.close();
}

void Logger::log(const std::string& message) {
    logQueue.enqueue(message);
}

// Function to get the current time as a formatted string
std::string Logger::getCurrentTime() {
    auto now = std::chrono::system_clock::now();
    auto time_point = std::chrono::system_clock::to_time_t(now);

    std::stringstream timeStream;
    timeStream << std::put_time(std::localtime(&time_point), "%Y-%m-%d %H:%M:%S");
    return timeStream.str();
}

void Logger::processLogs() {
    std::string message;
    while (running) {
        while (logQueue.try_dequeue(message)) {
            logFile << getCurrentTime() << " [LOG]: " << message << std::endl;
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(1)); // Avoid busy-spin
    }

    // Flush remaining logs
    while (logQueue.try_dequeue(message)) {
        logFile << getCurrentTime() << " [LOG]: " << message << std::endl;
    }
}
