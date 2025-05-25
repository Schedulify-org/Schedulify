#ifndef SCHEDULES_DISPLAY_H
#define SCHEDULES_DISPLAY_H

#include "controller_manager.h"
#include "main/model_access.h"
#include "model_interfaces.h"
#include <QObject>
#include <QVariant>
#include <QFileDialog>
#include <QDir>
#include <QQuickItem>
#include <QStandardPaths>
#include <QQuickItemGrabResult>
#include <QtQuick/QQuickItem>
#include <QDebug>

enum class fileType {
    PNG,
    CSV
};

// Filter structures
struct TimeRange {
    int startHour;
    int startMinute;
    int endHour;
    int endMinute;

    TimeRange(int sh = 0, int sm = 0, int eh = 23, int em = 59)
        : startHour(sh), startMinute(sm), endHour(eh), endMinute(em) {}
};

struct ScheduleFilters {
    TimeRange timeRange;                    // Preferred time range
    int maxGapsPerDay = -1;                 // Maximum gaps between classes (-1 = no limit)
    int minBreakMinutes = 0;                // Minimum break between classes
    int maxDaysToStudy = 7;
    int avgDayStartHour = -1, avgDayStartMinute = -1;
    int avgDayEndHour = -1, avgDayEndMinute = -1;

};

class SchedulesDisplayController : public ControllerManager {
    Q_OBJECT
    Q_PROPERTY(int currentScheduleIndex READ currentScheduleIndex
                            WRITE setCurrentScheduleIndex NOTIFY currentScheduleIndexChanged)

public:
    explicit SchedulesDisplayController(QObject *parent = nullptr);
    ~SchedulesDisplayController() override;
    void loadScheduleData(const vector<InformativeSchedule>& schedules);
    [[nodiscard]] int currentScheduleIndex() const;
    Q_INVOKABLE void setCurrentScheduleIndex(int index);

    Q_INVOKABLE [[nodiscard]] QVariantList getDayItems(int scheduleIndex, int dayIndex) const;
    Q_INVOKABLE [[nodiscard]] static QString getDayName(int dayIndex) ;
    Q_INVOKABLE [[nodiscard]] int getScheduleCount() const;
    Q_INVOKABLE void goBack() override;
    Q_INVOKABLE void saveScheduleAsCSV();
    Q_INVOKABLE void printScheduleDirectly();
    Q_INVOKABLE void captureAndSave(QQuickItem* item, const QString& savePath = QString());

    static QString generateFilename(const QString& basePath, int index, fileType type);

    // Filter methods
    Q_INVOKABLE void applyFilters(const QVariantMap& filters);
    Q_INVOKABLE void clearFilters();
    Q_INVOKABLE bool hasFilters() const;

signals:
    void currentScheduleIndexChanged();
    void scheduleCountChanged();
    void filtersChanged();
    void navigateBack();
    void screenshotSaved(const QString& path);
    void screenshotFailed();
    void scheduleChanged();

private:
    vector<InformativeSchedule> m_schedules;
    int m_currentScheduleIndex;
    IModel* modelConnection;
    std::vector<InformativeSchedule> m_allSchedules;   // המחסן המלא
    std::vector<int> m_filteredIndices; // אינדקסים שעברו סינון
    ScheduleFilters m_currentFilters;
    bool m_filtersActive;

    // Filter helper methods
    bool passesDayFilter(const InformativeSchedule&,
                    const std::vector<int>&) const;
    bool passesTimeFilter(const InformativeSchedule&,
                          const TimeRange&) const;
    bool passesBuildingFilter(const InformativeSchedule&,
                              const ScheduleFilters&) const;
    bool passesGapFilter(const InformativeSchedule&,
                         int maxGap, int minBreak) const;
    bool passesCompactnessFilter(const InformativeSchedule&) const;
    bool passesCourseTypeFilter(const InformativeSchedule&,const std::vector<std::string>&) const;
    static bool passesMaxDaysFilter(const InformativeSchedule& schedule, int maxDays) ;
    bool passesAllFilters(const InformativeSchedule& schedule,const ScheduleFilters& filters) const;

    static int calculateGapsInDay(const std::vector<ScheduleItem>& dayItems) ;
    void applyScheduleFilters();
    double avgDayStart(const InformativeSchedule& schedule);
    double avgDayEnd(const InformativeSchedule& schedule);

};

#endif // SCHEDULES_DISPLAY_H