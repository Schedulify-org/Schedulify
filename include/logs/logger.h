#ifndef LOGGER_H
#define LOGGER_H

#include <string>
#include <mutex>

enum class LogLevel {
    INFO,
    ERROR,
    WARNING
};

class Logger {
public:
    static Logger& get();

    void initialize(); // Sets timestamped log path
    void log(const string& message); // Basic message
    void log(LogLevel level, const string& message); // With level

    void logInfo(const string& message);
    void logError(const string& message);
    void logWarning(const string& message);

    string getLogPath() const;

private:
    Logger();
    string logPath;
    mutable std::mutex logMutex;
};

#endif // LOGGER_H
