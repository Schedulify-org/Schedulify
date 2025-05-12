#include "logger.h"
#include <mutex>

Logger::Logger() = default;

Logger& Logger::get() {
    static Logger instance;
    return instance;
}

void Logger::logInitiate() {
    log(LogLevel::INITIATE, "initiate application");
}

void Logger::log(LogLevel level, const string& message) {
    LogEntry entry;
    entry.timestamp = getTimeStamp();
    entry.level = level;
    entry.message = message;

    {
        std::lock_guard<std::mutex> lock(logMutex);
        logList.push_back(entry);
    }

    emit logAdded();
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

const vector<LogEntry>& Logger::getLogs() const {
    return logList;
}

string Logger::getTimeStamp() {
    time_t now = time(nullptr);
    tm* localTime = localtime(&now);

    ostringstream oss;
    oss << put_time(localTime, "%d/%m/%y-%H:%M:%S");
    return oss.str();
}
