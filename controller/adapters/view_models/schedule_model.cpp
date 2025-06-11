#include "schedule_model.h"

ScheduleModel::ScheduleModel(QObject *parent)
        : QObject(parent), m_currentScheduleIndex(0) {
}

void ScheduleModel::loadSchedules(const std::vector<InformativeSchedule>& schedules) {
    m_schedules = schedules;
    setCurrentScheduleIndex(0);

    emit scheduleCountChanged();
    emit scheduleDataChanged();
}

void ScheduleModel::setCurrentScheduleIndex(int index) {
    if (index >= 0 && index < static_cast<int>(m_schedules.size()) && m_currentScheduleIndex != index) {
        m_currentScheduleIndex = index;
        emit currentScheduleIndexChanged();
    }
}

QVariantList ScheduleModel::getDayItems(int scheduleIndex, int dayIndex) const {
    if (scheduleIndex < 0 || scheduleIndex >= static_cast<int>(m_schedules.size()))
        return {};

    if (dayIndex < 0 || dayIndex >= static_cast<int>(m_schedules[scheduleIndex].week.size()))
        return {};

    QVariantList items;
    for (const auto &item : m_schedules[scheduleIndex].week[dayIndex].day_items) {
        QVariantMap itemMap;
        itemMap["courseName"] = QString::fromStdString(item.courseName);
        itemMap["raw_id"] = QString::fromStdString(item.raw_id);
        itemMap["type"] = QString::fromStdString(item.type);
        itemMap["start"] = QString::fromStdString(item.start);
        itemMap["end"] = QString::fromStdString(item.end);
        itemMap["building"] = QString::fromStdString(item.building);
        itemMap["room"] = QString::fromStdString(item.room);
        items.append(itemMap);
    }
    return items;
}

QVariantList ScheduleModel::getCurrentDayItems(int dayIndex) const {
    return getDayItems(m_currentScheduleIndex, dayIndex);
}

void ScheduleModel::nextSchedule() {
    if (canGoNext()) {
        setCurrentScheduleIndex(m_currentScheduleIndex + 1);
    }
}

void ScheduleModel::previousSchedule() {
    if (canGoPrevious()) {
        setCurrentScheduleIndex(m_currentScheduleIndex - 1);
    }
}

bool ScheduleModel::canGoNext() const {
    return m_currentScheduleIndex < static_cast<int>(m_schedules.size()) - 1 && !m_schedules.empty();
}

bool ScheduleModel::canGoPrevious() const {
    return m_currentScheduleIndex > 0 && !m_schedules.empty();
}
void ScheduleModel::jumpToSchedule(int userScheduleNumber) {
    // Convert from 1-based user input to 0-based array index
    int index = userScheduleNumber - 1;

    if (canJumpToSchedule(index)) {
        setCurrentScheduleIndex(index);
    }
    // Remove the emit here - setCurrentScheduleIndex already emits it
}

bool ScheduleModel::canJumpToSchedule(int index) {
    // Fix: include both bounds checking
    return index >= 0 && index < static_cast<int>(m_schedules.size());
}