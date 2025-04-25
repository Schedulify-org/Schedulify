#ifndef BUTTONCONTROLLER_H
#define BUTTONCONTROLLER_H

#include <QObject>
#include <iostream>

class ButtonController : public QObject
{
Q_OBJECT

public:
    explicit ButtonController(QObject *parent = nullptr);
    virtual ~ButtonController(); // Add virtual destructor

public slots:
    // Slots to handle button clicks
    Q_INVOKABLE void handleButtonClicked(const QString &buttonName);
    Q_INVOKABLE void handleBrowseFiles();
    Q_INVOKABLE void handleUploadAndContinue();
};

#endif // BUTTONCONTROLLER_H