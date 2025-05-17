#ifndef BASE_CONTROLLER_H
#define BASE_CONTROLLER_H

#include <QObject>
#include <QUrl>
#include <QString>
#include <QDebug>
#include <QQmlApplicationEngine>

class MainController; // Forward declaration

class ControllerManager : public QObject {
Q_OBJECT

protected:
    static MainController* s_mainController;

public:
    explicit ControllerManager(QObject *parent = nullptr) : QObject(parent) {}

    // Set the main controller reference (call this from main.cpp)
    static void setMainController(MainController* controller);

    // Find another controller by name
    static QObject* findController(const QString& controllerName);

    static QQmlApplicationEngine* getEngine() ;

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