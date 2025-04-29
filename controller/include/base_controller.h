#ifndef BASE_CONTROLLER_H
#define BASE_CONTROLLER_H

#include <QObject>
#include <QUrl>

class BaseController : public QObject {
Q_OBJECT  // This macro is required for any class using signals/slots

public:
    explicit BaseController(QObject *parent = nullptr) : QObject(parent) {}

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