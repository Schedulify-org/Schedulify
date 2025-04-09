#include "logs/logManager.h"
#include "logs/logger.h"
#include <mutex>

Logger::Logger() = default;

Logger& Logger::get() {
    static Logger instance;
    return instance;
}

void Logger::initialize() {
    logPath = getTimestampedLogPath();
    createLogFile(logPath);
}

void Logger::log(const string& message) {
    std::lock_guard<std::mutex> lock(logMutex);
    appendToLogFile(logPath, message);
}
