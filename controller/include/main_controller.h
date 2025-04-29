#ifndef MAIN_CONTROLLER_H
#define MAIN_CONTROLLER_H

#include <QObject>
#include <QQmlApplicationEngine>
#include <QUrl>
#include "file_input.h"
#include "course_selection.h"
#include "schedules_display.h"

class MainController : public QObject {
Q_OBJECT

public:
    explicit MainController(QQmlApplicationEngine* engine, QObject *parent = nullptr);
    ~MainController() override = default;

    // Getter methods for sub-controllers
    [[nodiscard]] FileInputController* fileInputController() const { return m_fileInputController; }
    [[nodiscard]] CourseSelectionController* courseSelectionController() const { return m_courseSelectionController; }
    [[nodiscard]] SchedulesDisplayController* schedulesDisplayController() const { return m_schedulesDisplayController; }

signals:
    void navigateToScreen(const QUrl &screenUrl);
    void navigateBack();

private:
    QQmlApplicationEngine* m_engine;

    // Sub-controllers for each screen
    FileInputController* m_fileInputController;
    CourseSelectionController* m_courseSelectionController;
    SchedulesDisplayController* m_schedulesDisplayController;

    // Connect sub-controller signals to main controller signals
    void connectControllerSignals(BaseController* controller) const;
};

#endif // MAIN_CONTROLLER_H