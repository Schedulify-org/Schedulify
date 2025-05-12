#ifndef LOG_CONTROLLER_H
#define LOG_CONTROLLER_H

#include <QObject>
#include <QVariant>
#include <QVariantList>
#include <QColor>
#include <QDateTime>
#include <QJsonObject>
#include <QCoreApplication>
#include <QPointer>

#include "logger.h"
#include "controller_manager.h"

class LogDisplayController : public ControllerManager {
Q_OBJECT
    Q_PROPERTY(QVariantList logEntries READ getLogEntries NOTIFY logEntriesChanged)
    Q_PROPERTY(QString appLoadMessage READ getAppLoadMessage NOTIFY logEntriesChanged)

public:
    explicit LogDisplayController(QObject* parent = nullptr);
    virtual ~LogDisplayController() override = default;

    // Method to refresh logs
    Q_INVOKABLE void refreshLogs();
    Q_INVOKABLE void forceUpdate() {
        updateLogEntries();
        emit logEntriesChanged();
    }

    // Method to get log entries as QVariantList for QML
    QVariantList getLogEntries() const;

    // Method to get app loaded message
    QString getAppLoadMessage() const;

    // Helper method to convert LogLevel to QColor
    static QColor getColorForLogLevel(LogLevel level);

    // Helper method to convert LogLevel to string
    static QString getStringForLogLevel(LogLevel level);

signals:
    void logEntriesChanged();

private:
    void updateLogEntries();
    QVariantList m_logEntries;
    function<void()> m_logCallback;
};


#endif //LOG_CONTROLLER_H
