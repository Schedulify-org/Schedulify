#include "log_display.h"

LogDisplayController::LogDisplayController(QObject* parent) : ControllerManager(parent) {
    // Register for log updates
    Logger::get().registerLogCallback([this]() {
        QMetaObject::invokeMethod(this, "refreshLogs", Qt::QueuedConnection);
    });

    updateLogEntries();
}

void LogDisplayController::refreshLogs() {
    // Get fresh logs from the Logger
    updateLogEntries();

    // Explicitly emit the change signal to ensure QML updates
    emit logEntriesChanged();

    // Force QML to process events - may help with updates
    QCoreApplication::processEvents();
}

QVariantList LogDisplayController::getLogEntries() const {
    return m_logEntries;
}

QString LogDisplayController::getAppLoadMessage() const {
    if (!m_logEntries.isEmpty()) {
        const QVariantMap& firstLog = m_logEntries.first().toMap();
        if (firstLog["level"].toString() == "INITIATE") {
            return QString("Schedulify logs, app loaded on %1").arg(firstLog["timestamp"].toString());
        }
    }
    return "Schedulify logs";
}

QColor LogDisplayController::getColorForLogLevel(LogLevel level) {
    switch (level) {
        case LogLevel::INITIATE: return QColor("#0077CC"); // Blue
        case LogLevel::INFO:     return QColor("#00AA00"); // Green
        case LogLevel::WARNING:  return QColor("#FFAA00"); // Yellow
        case LogLevel::ERROR:    return QColor("#DD0000"); // Red
        default:                 return QColor("#888888"); // Gray
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