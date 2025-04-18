#include "logs/logManager.h"

namespace fs = std::filesystem;

string getTimestampedLogPath() {
    ostringstream oss;
    oss << "../data/logs/" << getTimeStamp() << ".log";

    return oss.str();
}

string getTimeStamp() {
    time_t now = time(nullptr);
    tm* localTime = localtime(&now);

    ostringstream oss;
    oss << put_time(localTime, "%d%m%y_%H-%M-%S");
    return oss.str();
}

void createLogFile(const string& path) {
    fs::create_directories("../data/logs");
    ofstream out(path);
    out.close();
}

void appendToLogFile(const string& logPath, const string& message) {
    ostringstream oss;
    oss << getTimeStamp() << ": " << message;
    string fullMessage = oss.str();

    ofstream out(logPath, ios::app);
    if (out.is_open()) {
        out << fullMessage << endl;
    }
    out.close();
}