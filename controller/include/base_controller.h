#ifndef BASE_CONTROLLER_H
#define BASE_CONTROLLER_H

#include <QObject>
#include <QUrl>
#include <QString>
#include <QDebug>

class MainController; // Forward declaration

class BaseController : public QObject {
Q_OBJECT  // This macro is required for any class using signals/slots

protected:
    static MainController* s_mainController; // Static reference to main controller

public:
    explicit BaseController(QObject *parent = nullptr) : QObject(parent) {}

    // Set the main controller reference (call this from main.cpp)
    static void setMainController(MainController* controller);

    // Find another controller by name
    static QObject* findController(const QString& controllerName);

signals:
    void navigateToScreen(const QUrl &screenUrl);
    void navigateBack();

public slots:
    virtual void goBack() {
        emit navigateBack();
    }

    virtual void goToScreen(const QUrl &screenUrl) {
        emit navigateToScreen(screenUrl);
    }
};

#endif // BASE_CONTROLLER_H