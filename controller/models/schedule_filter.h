#ifndef SCHEDULE_FILTER_H
#define SCHEDULE_FILTER_H

#include <QObject>
#include <vector>
#include "model_interfaces.h"

class ScheduleFilter : public QObject {
Q_OBJECT

public:
    explicit ScheduleFilter(QObject *parent = nullptr);
    ~ScheduleFilter() override = default;

    struct FilterCriteria {
        // Days to Study Filter (also serves as Active Days - max days with courses)
        bool daysToStudyEnabled = false;
        int daysToStudyValue = 7;
        
        // Total Gaps Filter
        bool totalGapsEnabled = false;
        int totalGapsValue = 0;
        
        // Max Gaps Time Filter
        bool maxGapsTimeEnabled = false;
        int maxGapsTimeValue = 90;
        
        // Average Day Start Filter
        bool avgDayStartEnabled = false;
        int avgDayStartHour = 8;
        int avgDayStartMinute = 0;
        
        // Average Day End Filter
        bool avgDayEndEnabled = false;
        int avgDayEndHour = 17;
        int avgDayEndMinute = 0;
    };

    // Main filtering method
    vector<InformativeSchedule> filterSchedules(const vector<InformativeSchedule>& schedules,
                                                const FilterCriteria& criteria);

    static int countActiveDays(const InformativeSchedule& schedule);
    static bool meetsDaysToStudyCriteria(const InformativeSchedule& schedule, const FilterCriteria& criteria);
    static bool meetsTotalGapsCriteria(const InformativeSchedule& schedule, const FilterCriteria& criteria);
    static bool meetsMaxGapsTimeCriteria(const InformativeSchedule& schedule, const FilterCriteria& criteria);
    static bool meetsAvgDayStartCriteria(const InformativeSchedule& schedule, const FilterCriteria& criteria);
    static bool meetsAvgDayEndCriteria(const InformativeSchedule& schedule, const FilterCriteria& criteria);

signals:
    void filteringStarted();
    void filteringFinished(int filteredCount, int totalCount);

private:
    // Helper methods
    static int timeToMinutes(int hour, int minute);
    static std::pair<int, int> minutesToTime(int minutes);
};

#endif // SCHEDULE_FILTER_H