#include "schedule_filter.h"

ScheduleFilter::ScheduleFilter(QObject *parent) : QObject(parent) {
}

std::vector<InformativeSchedule> ScheduleFilter::filterSchedules(const std::vector<InformativeSchedule>& schedules,
        const FilterCriteria& criteria) {

    emit filteringStarted();

    std::vector<InformativeSchedule> filtered;

    for (const auto& schedule : schedules) {
        bool passesAllFilters = true;

        // Check Days to Study Filter (Active Days)
        if (criteria.daysToStudyEnabled) {
            if (!meetsDaysToStudyCriteria(schedule, criteria)) {
                passesAllFilters = false;
            }
        }

        // Check Total Gaps Filter
        if (criteria.totalGapsEnabled && passesAllFilters) {
            if (!meetsTotalGapsCriteria(schedule, criteria)) {
                passesAllFilters = false;
            }
        }

        // Check Max Gaps Time Filter
        if (criteria.maxGapsTimeEnabled && passesAllFilters) {
            if (!meetsMaxGapsTimeCriteria(schedule, criteria)) {
                passesAllFilters = false;
            }
        }

        // Check Average Day Start Filter
        if (criteria.avgDayStartEnabled && passesAllFilters) {
            if (!meetsAvgDayStartCriteria(schedule, criteria)) {
                passesAllFilters = false;
            }
        }

        // Check Average Day End Filter
        if (criteria.avgDayEndEnabled && passesAllFilters) {
            if (!meetsAvgDayEndCriteria(schedule, criteria)) {
                passesAllFilters = false;
            }
        }

        if (passesAllFilters) {
            filtered.push_back(schedule);
        }
    }

    emit filteringFinished(static_cast<int>(filtered.size()), static_cast<int>(schedules.size()));

    return filtered;
}

int ScheduleFilter::countActiveDays(const InformativeSchedule& schedule) {
    int activeDays = 0;

    for (const auto& day : schedule.week) {
        if (!day.day_items.empty()) {
            activeDays++;
        }
    }

    return activeDays;
}

bool ScheduleFilter::meetsDaysToStudyCriteria(const InformativeSchedule& schedule, const FilterCriteria& criteria) {
    if (!criteria.daysToStudyEnabled) {
        return true;
    }

    // Count active days (days with courses) and check if it's <= max allowed
    int activeDays = countActiveDays(schedule);
    return activeDays <= criteria.daysToStudyValue;
}

bool ScheduleFilter::meetsTotalGapsCriteria(const InformativeSchedule& schedule, const FilterCriteria& criteria) {
    if (!criteria.totalGapsEnabled) {
        return true;
    }

    int totalGaps = 0;

    for (const auto& day : schedule.week) {
        if (day.day_items.size() <= 1) {
            continue; // No gaps possible with 0 or 1 items
        }

        // Sort items by start time for gap calculation
        std::vector<ScheduleItem> sortedItems = day.day_items;
        std::sort(sortedItems.begin(), sortedItems.end(), [](const ScheduleItem& a, const ScheduleItem& b) {
            return timeToMinutes(std::stoi(a.start.substr(0, 2)), std::stoi(a.start.substr(3, 2))) <
                   timeToMinutes(std::stoi(b.start.substr(0, 2)), std::stoi(b.start.substr(3, 2)));
        });

        // Count gaps between consecutive items
        for (size_t i = 0; i < sortedItems.size() - 1; ++i) {
            int currentEndTime = timeToMinutes(std::stoi(sortedItems[i].end.substr(0, 2)),
                                               std::stoi(sortedItems[i].end.substr(3, 2)));
            int nextStartTime = timeToMinutes(std::stoi(sortedItems[i+1].start.substr(0, 2)),
                                              std::stoi(sortedItems[i+1].start.substr(3, 2)));

            if (nextStartTime > currentEndTime) {
                totalGaps++;
            }
        }
    }

    return totalGaps <= criteria.totalGapsValue;
}

bool ScheduleFilter::meetsMaxGapsTimeCriteria(const InformativeSchedule& schedule, const FilterCriteria& criteria) {
    if (!criteria.maxGapsTimeEnabled) {
        return true;
    }

    int maxGapTime = 0;

    for (const auto& day : schedule.week) {
        if (day.day_items.size() <= 1) {
            continue;
        }

        // Sort items by start time
        std::vector<ScheduleItem> sortedItems = day.day_items;
        std::sort(sortedItems.begin(), sortedItems.end(), [](const ScheduleItem& a, const ScheduleItem& b) {
            return timeToMinutes(std::stoi(a.start.substr(0, 2)), std::stoi(a.start.substr(3, 2))) <
                   timeToMinutes(std::stoi(b.start.substr(0, 2)), std::stoi(b.start.substr(3, 2)));
        });

        // Find maximum gap time
        for (size_t i = 0; i < sortedItems.size() - 1; ++i) {
            int currentEndTime = timeToMinutes(std::stoi(sortedItems[i].end.substr(0, 2)),
                                               std::stoi(sortedItems[i].end.substr(3, 2)));
            int nextStartTime = timeToMinutes(std::stoi(sortedItems[i+1].start.substr(0, 2)),
                                              std::stoi(sortedItems[i+1].start.substr(3, 2)));

            if (nextStartTime > currentEndTime) {
                int gapTime = nextStartTime - currentEndTime;
                maxGapTime = std::max(maxGapTime, gapTime);
            }
        }
    }

    return maxGapTime <= criteria.maxGapsTimeValue;
}

bool ScheduleFilter::meetsAvgDayStartCriteria(const InformativeSchedule& schedule, const FilterCriteria& criteria) {
    if (!criteria.avgDayStartEnabled) {
        return true;
    }

    int totalStartTime = 0;
    int activeDays = 0;

    for (const auto& day : schedule.week) {
        if (day.day_items.empty()) {
            continue;
        }

        // Find earliest start time for this day
        int earliestStart = 24 * 60; // 24 hours in minutes
        for (const auto& item : day.day_items) {
            int startTime = timeToMinutes(std::stoi(item.start.substr(0, 2)),
                                          std::stoi(item.start.substr(3, 2)));
            earliestStart = std::min(earliestStart, startTime);
        }

        totalStartTime += earliestStart;
        activeDays++;
    }

    if (activeDays == 0) {
        return true; // No classes, so criteria doesn't apply
    }

    int avgStartTime = totalStartTime / activeDays;
    int criteriaStartTime = timeToMinutes(criteria.avgDayStartHour, criteria.avgDayStartMinute);

    // Schedule should start after or at the specified time
    return avgStartTime >= criteriaStartTime;
}

bool ScheduleFilter::meetsAvgDayEndCriteria(const InformativeSchedule& schedule, const FilterCriteria& criteria) {
    if (!criteria.avgDayEndEnabled) {
        return true;
    }

    int totalEndTime = 0;
    int activeDays = 0;

    for (const auto& day : schedule.week) {
        if (day.day_items.empty()) {
            continue;
        }

        // Find latest end time for this day
        int latestEnd = 0;
        for (const auto& item : day.day_items) {
            int endTime = timeToMinutes(std::stoi(item.end.substr(0, 2)),
                                        std::stoi(item.end.substr(3, 2)));
            latestEnd = std::max(latestEnd, endTime);
        }

        totalEndTime += latestEnd;
        activeDays++;
    }

    if (activeDays == 0) {
        return true; // No classes, so criteria doesn't apply
    }

    int avgEndTime = totalEndTime / activeDays;
    int criteriaEndTime = timeToMinutes(criteria.avgDayEndHour, criteria.avgDayEndMinute);

    // Schedule should end before or at the specified time
    return avgEndTime <= criteriaEndTime;
}

// Helper methods
int ScheduleFilter::timeToMinutes(int hour, int minute) {
    return hour * 60 + minute;
}

std::pair<int, int> ScheduleFilter::minutesToTime(int minutes) {
    return std::make_pair(minutes / 60, minutes % 60);
}