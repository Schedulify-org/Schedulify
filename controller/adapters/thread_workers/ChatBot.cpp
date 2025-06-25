#include "ChatBot.h"
#include <QDebug>
#include <iostream>

BotWorker::BotWorker(IModel* model, const std::vector<std::string>& messageData, QObject* parent)
        : QObject(parent), m_model(model), m_messageData(messageData) {
}

void BotWorker::processMessage() {
    try {
        if (!m_model) {
            emit errorOccurred("Model connection not available");
            emit finished();
            return;
        }

        // Send to model using BOT_MESSAGE operation
        void* result = m_model->executeOperation(ModelOperation::BOT_MESSAGE, &m_messageData, "");

        if (result) {
            auto* responseVector = static_cast<std::vector<std::string>*>(result);

            if (!responseVector->empty()) {
                QString responseText = QString::fromStdString((*responseVector)[0]);

                // Handle optional index in second cell
                int scheduleIndex = -1;

                if (responseVector->size() >= 2) {
                    const std::string& indexStr = (*responseVector)[1];

                    if (!indexStr.empty() && indexStr != "-1") {
                        try {
                            scheduleIndex = std::stoi(indexStr);
                            qDebug() << "BotWorker: Received schedule index:" << scheduleIndex;
                        } catch (const std::exception& e) {
                            qDebug() << "BotWorker: Invalid index format:" << QString::fromStdString(indexStr);
                        }
                    }
                }

                emit responseReady(responseText, scheduleIndex);
            } else {
                emit errorOccurred("I processed your request but don't have a response at the moment.");
            }

            delete responseVector; // Clean up the allocated memory
        } else {
            emit errorOccurred("I'm sorry, I couldn't process your request. Please try rephrasing your question.");
        }

    } catch (const std::exception& e) {
        qWarning() << "BotWorker: Error processing message:" << e.what();
        emit errorOccurred("An error occurred while processing your request. Please try again.");
    } catch (...) {
        qWarning() << "BotWorker: Unknown error processing message";
        emit errorOccurred("An unexpected error occurred. Please try again.");
    }

    emit finished();
}