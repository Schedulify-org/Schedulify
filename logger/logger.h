#ifndef LOGGER_H
#define LOGGER_H

#include <string>
#include <vector>
#include <mutex>
#include <sstream>
#include <iomanip>
#include <fstream>
#include <iostream>
#include <functional>
#include <QObject>

using std::string;
using std::vector;
using std::ostringstream;
using std::put_time;
using std::ofstream;
using std::endl;
using std::ios;
using std::function;

enum class LogLevel {
    INFO,
    ERROR,
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
    void log(LogLevel level, const string& message);
    static string getTimeStamp();

    vector<LogEntry> logList;
    mutable std::mutex logMutex;

    vector<std::function<void()>> logCallbacks;
};

#endif // LOGGER_H
