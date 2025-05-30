#ifndef LOGGER_H
#define LOGGER_H

#include <string>
#include <vector>
#include <mutex>
#include <iomanip>
#include <fstream>
#include <QObject>
#include <sstream>

using std::string;
using std::vector;
using std::ostringstream;
using std::put_time;
using std::endl;

enum class LogLevel {
    INFO,
    ERR,
    WARNING,
    INITIATE
};

struct LogEntry {
    string timestamp;
    LogLevel level;
    string message;
};

class Logger : public QObject {
Q_OBJECT
public:
    static Logger& get();
    void logInitiate();

    void logInfo(const string& message);
    void logError(const string& message);
    void logWarning(const string& message);

    const vector<LogEntry>& getLogs() const;

signals:
    void logAdded();

private:
    Logger();

    Logger(const Logger&) = delete;
    Logger& operator=(const Logger&) = delete;
    Logger(Logger&&) = delete;
    Logger& operator=(Logger&&) = delete;

    void log(LogLevel level, const string& message);
    static string getTimeStamp();

    vector<LogEntry> logList;
    mutable std::mutex logMutex;
};

#endif // LOGGER_H
