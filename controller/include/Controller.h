#ifndef BUTTONCONTROLLER_H
#define BUTTONCONTROLLER_H

#include <QObject>
#include <iostream>
#include <QQmlApplicationEngine>
#include "CourseModel.h"

class ButtonController : public QObject
{
Q_OBJECT
    Q_PROPERTY(CourseModel* courseModel READ courseModel CONSTANT)

public:
    explicit ButtonController(QQmlApplicationEngine* engine, QObject *parent = nullptr);
    virtual ~ButtonController();

    // Getter for courseModel property
    CourseModel* courseModel() const { return m_courseModel; }

signals:
    // Navigation signals
    void navigateToScreen(const QUrl &screenUrl);
    void navigateBack();

public slots:
    // Slots to handle button clicks
    Q_INVOKABLE void handleButtonClicked(const QString &buttonName);
    Q_INVOKABLE void handleBrowseFiles();
    Q_INVOKABLE void handleUploadAndContinue();

    // Navigation methods
    Q_INVOKABLE void goToCourseListScreen();
    Q_INVOKABLE void goBack();

private:
    QQmlApplicationEngine* m_engine;
    CourseModel* m_courseModel;
};

#endif // BUTTONCONTROLLER_H