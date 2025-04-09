// Logger.h
#ifndef LOGGER_H
#define LOGGER_H

#include <string>
#include <mutex>

class Logger {
public:
    static Logger& get();

    void initialize(); // Uses getTimestampedLogPath internally
    void log(const std::string& message);


private:
    Logger();

    std::string logPath;
    mutable std::mutex logMutex;
};

#endif // LOGGER_H
