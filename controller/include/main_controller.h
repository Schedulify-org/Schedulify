// include/main_controller.h (updated)
#ifndef MAIN_CONTROLLER_H
#define MAIN_CONTROLLER_H

#include "file_input.h"
#include "course_selection.h"
#include "schedules_display.h"
#include "log_display.h"

#include <QObject>
#include <QQmlApplicationEngine>
#include <QUrl>

class MainController : public QObject {
Q_OBJECT

public:
    explicit MainController(QQmlApplicationEngine* engine, QObject *parent = nullptr);

    FileInputController* fileInputController() const { return m_fileInputController; }
    CourseSelectionController* courseSelectionController() const { return m_courseSelectionController; }
    SchedulesDisplayController* schedulesDisplayController() const { return m_schedulesDisplayController; }
    LogDisplayController* logDisplayController() const { return m_logDisplayController; }
    QQmlApplicationEngine* getEngine() const { return m_engine; }

signals:
    void navigateToScreen(const QUrl &screenUrl);
    void navigateBack();

private:
    QQmlApplicationEngine* m_engine;

    // Sub-controllers for each screen
    FileInputController* m_fileInputController;
    CourseSelectionController* m_courseSelectionController;
    SchedulesDisplayController* m_schedulesDisplayController;
    LogDisplayController* m_logDisplayController;

    // Connect sub-controller signals to main controller signals
    void connectControllerSignals(const ControllerManager* controller) const;
};

#endif // MAIN_CONTROLLER_H
