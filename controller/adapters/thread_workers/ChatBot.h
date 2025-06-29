#ifndef CHAT_BOT_H
#define CHAT_BOT_H

#include <QObject>
#include <QThread>
#include <QString>
#include <vector>
#include <string>
#include "model_interfaces.h"

class BotWorker : public QObject {
Q_OBJECT

public:
    // Legacy constructor for backward compatibility
    explicit BotWorker(IModel* model, const std::vector<std::string>& messageData, QObject* parent = nullptr);

    // New constructor for bot query requests
    explicit BotWorker(IModel* model, BotQueryRequest  queryRequest, QObject* parent = nullptr);

public slots:
    void processMessage();

signals:
    // New signal for structured bot query responses
    void responseReady(const BotQueryResponse& response);

    // Legacy signals for backward compatibility
    void responseReady(const QString& response, int scheduleIndex);
    void errorOccurred(const QString& error);
    void finished();

private:
    IModel* m_model;
    std::vector<std::string> m_messageData;  // Legacy format
    BotQueryRequest m_queryRequest;
    bool m_useLegacyFormat;

    // Helper methods
    void processBotQuery();
    static BotQueryResponse parseLegacyResponse(const std::vector<std::string>& responseVector);
};

#endif // CHAT_BOT_H