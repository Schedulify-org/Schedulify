#ifndef MAIN_CONTROLLER_H
#define MAIN_CONTROLLER_H

#include <QObject>
#include <iostream>
#include <QQmlApplicationEngine>
#include "CourseModel.h"

class MainController : public QObject
{
Q_OBJECT
    Q_PROPERTY(CourseModel* courseModel READ courseModel CONSTANT)

public:
    explicit MainController(QQmlApplicationEngine* engine, QObject *parent = nullptr);
    ~MainController() override = default;

    // Getter for courseModel property
    CourseModel* courseModel() const { return m_courseModel; }

signals:
    // Navigation signals
    void navigateToScreen(const QUrl &screenUrl);
    void navigateBack();

public slots:
    // Slots to handle button clicks
    Q_INVOKABLE void handleUploadAndContinue();

    // Navigation methods
    Q_INVOKABLE void goBack();
    Q_INVOKABLE void goToScreen(const QUrl &screenUrl);

private:
    QQmlApplicationEngine* m_engine;
    CourseModel* m_courseModel;
};

#endif // MAIN_CONTROLLER_H