#ifndef COURSE_SELECTION_H
#define COURSE_SELECTION_H

#include "course_model.h"
#include "ScheduleGenerator.h"
#include "CourseValidator.h"
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
    Q_PROPERTY(bool validationInProgress READ validationInProgress NOTIFY validationStateChanged)
    Q_PROPERTY(QStringList validationErrors READ validationErrors NOTIFY validationStateChanged)

public:
    explicit CourseSelectionController(QObject *parent = nullptr);
    ~CourseSelectionController() override;

    [[nodiscard]] CourseModel* courseModel() const { return m_courseModel; }
    [[nodiscard]] CourseModel* selectedCoursesModel() const { return m_selectedCoursesModel; }
    [[nodiscard]] CourseModel* filteredCourseModel() const { return m_filteredCourseModel; }
    [[nodiscard]] CourseModel* blocksModel() const { return m_blocksModel; }
    [[nodiscard]] bool validationInProgress() const { return m_validationInProgress; }
    [[nodiscard]] QStringList validationErrors() const { return m_validationErrors; }

    void initiateCoursesData(const vector<Course>& courses);

    Q_INVOKABLE bool isCourseSelected(int index);
    Q_INVOKABLE void toggleCourseSelection(int index);
    Q_INVOKABLE void filterCourses(const QString &text);
    Q_INVOKABLE void resetFilter();
    Q_INVOKABLE void generateSchedules();
    Q_INVOKABLE void deselectCourse(int index);
    Q_INVOKABLE void createNewCourse(const QString& courseName, const QString& courseId,
                                     const QString& teacherName, const QVariantList& sessionGroups);

    Q_INVOKABLE void addBlockTime(const QString& day, const QString& startTime, const QString& endTime);
    Q_INVOKABLE void removeBlockTime(int index);
    Q_INVOKABLE void clearAllBlockTimes();

    Q_INVOKABLE void setupValidationTimeout(int timeoutMs);

private slots:
    void onSchedulesGenerated(vector<InformativeSchedule>* schedules);
    void onValidationTimeout();

signals:
    void selectionChanged();
    void blockTimesChanged();
    void errorMessage(const QString &message);
    void validationStateChanged();

private:
    CourseModel* m_courseModel;
    CourseModel* m_selectedCoursesModel;
    CourseModel* m_filteredCourseModel;
    CourseModel* m_blocksModel;
    QTimer* validationTimeoutTimer = nullptr;
    bool validationCompleted = false;
    bool m_validationInProgress = false;
    QStringList m_validationErrors;
    vector<Course> allCourses;
    vector<Course> selectedCourses;
    vector<Course> filteredCourses;
    vector<Course> blockTimes;
    vector<BlockTime> userBlockTimes;

    vector<int> selectedIndices;
    vector<int> filteredIndicesMap;
    QString currentSearchText;
    IModel* modelConnection;
    QThread* validatorThread = nullptr;
    QThread* workerThread = nullptr;

    void updateBlockTimesModel();
    Course createSingleBlockTimeCourse();
    static int getDayNumber(const QString& dayName);
    Course createCourseFromData(const QString& courseName, const QString& courseId,
                                const QString& teacherName, const QVariantList& sessionGroups);
    void validateCourses(const vector<Course>& courses, int timeoutMs);
    void onCoursesValidated(vector<string>* errors);
    void cleanupValidation();
    void cleanupValidatorThread();
    void setValidationInProgress(bool inProgress);
    void setValidationErrors(const QStringList& errors);

    static const int VALIDATION_TIMEOUT_MS = 60000;
    static const int THREAD_CLEANUP_TIMEOUT_MS = 10000;
    static const int MAX_COURSES_LIMIT = 1000;
};

#endif //COURSE_SELECTION_H