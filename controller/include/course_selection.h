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

// Struct to represent a block time
struct BlockTime {
    QString day;
    QString startTime;
    QString endTime;

    BlockTime(const QString& d, const QString& st, const QString& et)
            : day(d), startTime(st), endTime(et) {}
};

class CourseSelectionController final : public ControllerManager {
Q_OBJECT

    Q_PROPERTY(CourseModel* courseModel READ courseModel CONSTANT)
    Q_PROPERTY(CourseModel* selectedCoursesModel READ selectedCoursesModel CONSTANT)
    Q_PROPERTY(CourseModel* filteredCourseModel READ filteredCourseModel CONSTANT)
    Q_PROPERTY(CourseModel* blocksModel READ blocksModel CONSTANT)

public:
    explicit CourseSelectionController(QObject *parent = nullptr);
    ~CourseSelectionController() override;

    [[nodiscard]] CourseModel* courseModel() const { return m_courseModel; }
    [[nodiscard]] CourseModel* selectedCoursesModel() const { return m_selectedCoursesModel; }
    [[nodiscard]] CourseModel* filteredCourseModel() const { return m_filteredCourseModel; }
    [[nodiscard]] CourseModel* blocksModel() const { return m_blocksModel; }

    void initiateCoursesData(const vector<Course>& courses);

    Q_INVOKABLE bool isCourseSelected(int index);
    Q_INVOKABLE void toggleCourseSelection(int index);
    Q_INVOKABLE void filterCourses(const QString &text);
    Q_INVOKABLE void resetFilter();
    Q_INVOKABLE void generateSchedules();
    Q_INVOKABLE void deselectCourse(int index);

    // Block time management methods
    Q_INVOKABLE void addBlockTime(const QString& day, const QString& startTime, const QString& endTime);
    Q_INVOKABLE void removeBlockTime(int index);
    Q_INVOKABLE void clearAllBlockTimes();

private slots:
    void onSchedulesGenerated(vector<InformativeSchedule>* schedules);

signals:
    void selectionChanged();
    void blockTimesChanged();
    void errorMessage(const QString &message);

private:
    CourseModel* m_courseModel;
    CourseModel* m_selectedCoursesModel;
    CourseModel* m_filteredCourseModel;
    CourseModel* m_blocksModel;

    vector<Course> allCourses;
    vector<Course> selectedCourses;
    vector<Course> filteredCourses;
    vector<Course> blockTimes; // This will store the block times as Course objects for display
    vector<BlockTime> userBlockTimes; // This stores the actual block time data

    vector<int> selectedIndices;
    vector<int> filteredIndicesMap;
    IModel* modelConnection;
    QThread* workerThread = nullptr;

    // Helper methods
    void updateBlockTimesModel();
    Course createBlockTimeCourse(const BlockTime& blockTime, int id);
    static int getDayNumber(const QString& dayName);
    static Group createBlockGroup(const BlockTime& blockTime);
};

#endif //COURSE_SELECTION_H