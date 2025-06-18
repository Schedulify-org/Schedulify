#ifndef SCHEDULES_DISPLAY_H
#define SCHEDULES_DISPLAY_H

#include "controller_manager.h"
#include "model_access.h"
#include "model_interfaces.h"
#include "schedule_model.h"

#include <QObject>
#include <QVariant>
#include <QVariantMap>
#include <QVariantList>
#include <QFileDialog>
#include <QDir>
#include <QQuickItem>
#include <QStandardPaths>
#include <QQuickItemGrabResult>
#include <QtQuick/QQuickItem>

enum class fileType {
    PNG,
    CSV
};

class SchedulesDisplayController : public ControllerManager {
Q_OBJECT
    Q_PROPERTY(ScheduleModel* scheduleModel READ scheduleModel CONSTANT)

public:
    explicit SchedulesDisplayController(QObject *parent = nullptr);
    ~SchedulesDisplayController() override;

    // UPDATED: Keep old method for backward compatibility during transition
    void loadScheduleData(const std::vector<InformativeSchedule>& schedules);

    // NEW: Semester-specific methods
    Q_INVOKABLE void loadSemesterScheduleData(const QString& semester, const std::vector<InformativeSchedule>& schedules);
    Q_INVOKABLE void switchToSemester(const QString& semester);
    Q_INVOKABLE void allSemestersGenerated();

    // Properties
    ScheduleModel* scheduleModel() const { return m_scheduleModel; }

    // NEW: Semester query methods
    Q_INVOKABLE QString getCurrentSemester() const { return m_currentSemester; }
    Q_INVOKABLE bool hasSchedulesForSemester(const QString& semester) const;
    Q_INVOKABLE int getScheduleCountForSemester(const QString& semester) const;

    // QML accessible methods
    Q_INVOKABLE void goBack() override;
    Q_INVOKABLE void saveScheduleAsCSV();
    Q_INVOKABLE void printScheduleDirectly();
    Q_INVOKABLE void captureAndSave(QQuickItem* item, const QString& savePath = QString());

    // Sorting methods
    Q_INVOKABLE void applySorting(const QVariantMap& sortData);
    Q_INVOKABLE void clearSorting();

    // UPDATED: Include semester in filename
    static QString generateFilename(const QString& basePath, int index, fileType type, const QString& semester = "");

signals:
    void schedulesSorted(int totalCount);
    void screenshotSaved(const QString& path);
    void screenshotFailed();

    // NEW: Semester-specific signals
    void currentSemesterChanged();
    void semesterSchedulesLoaded(const QString& semester);
    void allSemestersReady();

private:
    // UPDATED: Replace single schedule vector with semester-specific vectors
    std::vector<InformativeSchedule> m_schedulesA;
    std::vector<InformativeSchedule> m_schedulesB;
    std::vector<InformativeSchedule> m_schedulesSummer;

    // NEW: Semester management properties
    QString m_currentSemester = "A"; // Track which semester is currently being displayed
    bool m_allSemestersLoaded = false;

    ScheduleModel* m_scheduleModel;
    IModel* modelConnection;
    QMap<QString, QString> m_sortKeyMap;

    // Track current sort state for optimization
    QString m_currentSortField;
    bool m_currentSortAscending = true;

    // NEW: Helper methods
    std::vector<InformativeSchedule>* getCurrentScheduleVector();
};

#endif // SCHEDULES_DISPLAY_H