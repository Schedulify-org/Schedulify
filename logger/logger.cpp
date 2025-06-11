#include "logger.h"

Logger::Logger() : QObject(nullptr) {}

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

    // Collect messages if collection is enabled - use separate lock
    {
        std::lock_guard<std::mutex> lock(collectionMutex);
        if (collectingEnabled) {
            try {
                if (level == LogLevel::WARNING) {
                    collectedWarnings.push_back(message);
                } else if (level == LogLevel::ERR) {
                    collectedErrors.push_back(message);
                }
            } catch (const std::exception& e) {
                // Don't let collection errors crash the logger
                collectingEnabled = false;
            }
        }
    }

    emit logAdded();
}

void Logger::logInfo(const string& message) {
    log(LogLevel::INFO, message);
}

void Logger::logError(const string& message) {
    log(LogLevel::ERR, message);
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

void Logger::startCollecting() {
    std::lock_guard<std::mutex> lock(collectionMutex);
    collectingEnabled = true;
    collectedWarnings.clear();
    collectedErrors.clear();
}

void Logger::stopCollecting() {
    std::lock_guard<std::mutex> lock(collectionMutex);
    collectingEnabled = false;
}

void Logger::clearCollected() {
    std::lock_guard<std::mutex> lock(collectionMutex);
    collectedWarnings.clear();
    collectedErrors.clear();
}

vector<string> Logger::getCollectedWarnings() const {
    std::lock_guard<std::mutex> lock(collectionMutex);
    return collectedWarnings;
}

vector<string> Logger::getCollectedErrors() const {
    std::lock_guard<std::mutex> lock(collectionMutex);
    return collectedErrors;
}

vector<string> Logger::getAllCollectedMessages() const {
    std::lock_guard<std::mutex> lock(collectionMutex);
    vector<string> allMessages;

    // Add warnings with prefix
    for (const auto& warning : collectedWarnings) {
        allMessages.push_back("[Parser Warning] " + warning);
    }

    // Add errors with prefix
    for (const auto& error : collectedErrors) {
        allMessages.push_back("[Parser Error] " + error);
    }

    return allMessages;
}

bool Logger::isCollecting() const {
    std::lock_guard<std::mutex> lock(collectionMutex);
    return collectingEnabled;
}