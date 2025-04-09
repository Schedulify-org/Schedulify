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

void Logger::log(LogLevel level, const string& message) {
    std::string label;
    switch (level) {
        case LogLevel::INFO:
            label = "[INFO   ] ";
            break;
        case LogLevel::ERROR:
            label = "[ERROR  ] ";
            break;
        case LogLevel::WARNING:
            label = "[WARNING] ";
            break;
    }

    log(label + message);
}

void Logger::logInfo(const string& message) {
    log(LogLevel::INFO, message);
}

void Logger::logError(const string& message) {
    log(LogLevel::ERROR, message);
}

void Logger::logWarning(const string &message) {
    log(LogLevel::WARNING, message);
}

std::string Logger::getLogPath() const {
    return logPath;
}
