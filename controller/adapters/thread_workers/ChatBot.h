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
    explicit BotWorker(IModel* model, const std::vector<std::string>& messageData, QObject* parent = nullptr);

public slots:
    void processMessage();

signals:
    void responseReady(const QString& response, int scheduleIndex);
    void errorOccurred(const QString& error);
    void finished();

private:
    IModel* m_model;
    std::vector<std::string> m_messageData;
};

#endif // CHAT_BOT_H