#ifndef COURSE_SELECTION_H
#define COURSE_SELECTION_H

#include "course_model.h"
#include "ScheduleGenerator.h"
#include "model_access.h"
#include "controller_manager.h"
#include "schedules_display.h"
#include "logger.h"

#include <algorithm>
#include <QUrl>
#include <QStringList>
#include <QThread>
#include <vector>
#include <QQmlApplicationEngine>
#include <QQmlContext>
#include <QGuiApplication>
#include <QQuickWindow>
#include <QTimer>


class CourseSelectionController final : public ControllerManager {
Q_OBJECT

    Q_PROPERTY(CourseModel* courseModel READ courseModel CONSTANT)
    Q_PROPERTY(CourseModel* selectedCoursesModel READ selectedCoursesModel CONSTANT)
    Q_PROPERTY(CourseModel* filteredCourseModel READ filteredCourseModel CONSTANT)

public:
    explicit CourseSelectionController(QObject *parent = nullptr);
    ~CourseSelectionController() override;

    [[nodiscard]] CourseModel* courseModel() const { return m_courseModel; }
    [[nodiscard]] CourseModel* selectedCoursesModel() const { return m_selectedCoursesModel; }
    [[nodiscard]] CourseModel* filteredCourseModel() const { return m_filteredCourseModel; }

    void initiateCoursesData(const vector<Course>& courses);

    Q_INVOKABLE bool isCourseSelected(int index);
    Q_INVOKABLE void toggleCourseSelection(int index);
    Q_INVOKABLE void filterCourses(const QString &text);
    Q_INVOKABLE void resetFilter();
    Q_INVOKABLE void generateSchedules();
    Q_INVOKABLE void deselectCourse(int index);

private slots:
    void onSchedulesGenerated(std::vector<InformativeSchedule>* schedules);

signals:
    void selectionChanged();
    void errorMessage(const QString &message);

private:
    CourseModel* m_courseModel;
    CourseModel* m_selectedCoursesModel;
    CourseModel* m_filteredCourseModel;
    vector<Course> allCourses;
    vector<Course> selectedCourses;
    vector<Course> filteredCourses;
    vector<int> selectedIndices;
    vector<int> filteredIndicesMap;
    IModel* modelConnection;
    QThread* workerThread = nullptr;
};

#endif //COURSE_SELECTION_H