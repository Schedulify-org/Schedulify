#ifndef SCHEDULES_DISPLAY_H
#define SCHEDULES_DISPLAY_H

#include "controller_manager.h"
#include "main/model_access.h"
#include "model_interfaces.h"
#include "models/schedule_model.h"
#include "models/schedule_filter.h"


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

// Forward declaration
class ScheduleFilter;

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

    void loadScheduleData(const std::vector<InformativeSchedule>& schedules);

    // Properties
    ScheduleModel* scheduleModel() const { return m_scheduleModel; }

    // Existing QML accessible methods
    Q_INVOKABLE void goBack() override;
    Q_INVOKABLE void saveScheduleAsCSV();
    Q_INVOKABLE void printScheduleDirectly();
    Q_INVOKABLE void captureAndSave(QQuickItem* item, const QString& savePath = QString());

    Q_INVOKABLE void applyFiltersAndBlockedTimes(const QVariantMap& filterData, const QVariantList& blockedTimes);
    Q_INVOKABLE void clearFilters();

    static QString generateFilename(const QString& basePath, int index, fileType type);

signals:
    void filtersApplied(int filteredCount, int totalCount);
    void screenshotSaved(const QString& path);
    void screenshotFailed();

private:
    std::vector<InformativeSchedule> m_filteredSchedules;
    std::vector<InformativeSchedule> m_originalSchedules;
    ScheduleFilter* m_scheduleFilter;
    ScheduleModel* m_scheduleModel;
    IModel* modelConnection;

    ScheduleFilter::FilterCriteria convertQVariantToFilterCriteria(const QVariantMap& filterData);
    void applyFiltersToSchedules(const ScheduleFilter::FilterCriteria& criteria);
};

#endif // SCHEDULES_DISPLAY_H