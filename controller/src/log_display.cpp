#include "log_display.h"

LogDisplayController::LogDisplayController(QObject* parent) : ControllerManager(parent) {
    connect(&Logger::get(), &Logger::logAdded, this, &LogDisplayController::refreshLogs);
    updateLogEntries();
}

void LogDisplayController::refreshLogs() {
    updateLogEntries();

    emit logEntriesChanged();

    QCoreApplication::processEvents();
}

QVariantList LogDisplayController::getLogEntries() const {
    return m_logEntries;
}

QColor LogDisplayController::getColorForLogLevel(LogLevel level) {
    switch (level) {
        case LogLevel::INITIATE: return {"#0077CC"};
        case LogLevel::INFO:     return {"#00AA00"};
        case LogLevel::WARNING:  return {"#FFAA00"};
        case LogLevel::ERROR:    return {"#DD0000"};
        default:                 return {"#888888"};
    }
}

QString LogDisplayController::getStringForLogLevel(LogLevel level) {
    switch (level) {
        case LogLevel::INITIATE: return "INITIATE";
        case LogLevel::INFO:     return "INFO";
        case LogLevel::WARNING:  return "WARNING";
        case LogLevel::ERROR:    return "ERROR";
        default:                 return "UNKNOWN";
    }
}

void LogDisplayController::updateLogEntries() {
    m_logEntries.clear();

    const auto& logs = Logger::get().getLogs();

    for (const auto& entry : logs) {
        QVariantMap logMap;
        logMap["timestamp"] = QString::fromStdString(entry.timestamp);
        logMap["level"] = getStringForLogLevel(entry.level);
        logMap["message"] = QString::fromStdString(entry.message);
        logMap["color"] = getColorForLogLevel(entry.level);

        m_logEntries.append(logMap);
    }
}