#include "schedules_display.h"
#include <QQmlContext>
#include <QQmlEngine>

ScheduleListModel::ScheduleListModel(QObject *parent) : QAbstractListModel(parent) {
}

void ScheduleListModel::setSchedules(const vector<InformativeSchedule> &schedules) {
    beginResetModel();
    m_schedules = schedules;
    endResetModel();
}

int ScheduleListModel::rowCount(const QModelIndex &parent) const {
    if (parent.isValid())
        return 0;
    return static_cast<int>(m_schedules.size());
}

QVariant ScheduleListModel::data(const QModelIndex &index, int role) const {
    if (!index.isValid() || index.row() >= static_cast<int>(m_schedules.size()))
        return QVariant();

    const InformativeSchedule &schedule = m_schedules[index.row()];

    switch (role) {
        case IndexRole:
            return QVariant(schedule.index);
        case DaysRole: {
            QVariantList days;
            for (const auto &day : schedule.week) {
                QVariantMap dayMap;
                dayMap["day"] = QString::fromStdString(day.day);

                QVariantList items;
                for (const auto &item : day.day_items) {
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
                dayMap["items"] = items;
                days.append(dayMap);
            }
            return days;
        }
        default:
            return QVariant();
    }
}

QHash<int, QByteArray> ScheduleListModel::roleNames() const {
    QHash<int, QByteArray> roles;
    roles[IndexRole] = "index";
    roles[DaysRole] = "days";
    return roles;
}

SchedulesDisplayController::SchedulesDisplayController(QObject *parent)
    : ControllerManager(parent), m_currentScheduleIndex(0) {
    m_schedulesModel = new ScheduleListModel(this);
}

SchedulesDisplayController::~SchedulesDisplayController() {
}

void SchedulesDisplayController::loadScheduleData(const vector<InformativeSchedule> &schedules) {
    m_schedules = schedules;
    m_schedulesModel->setSchedules(schedules);
    setCurrentScheduleIndex(0);
}

ScheduleListModel* SchedulesDisplayController::schedulesModel() const {
    return m_schedulesModel;
}

int SchedulesDisplayController::currentScheduleIndex() const {
    return m_currentScheduleIndex;
}

void SchedulesDisplayController::setCurrentScheduleIndex(int index) {
    if (index >= 0 && index < static_cast<int>(m_schedules.size()) && m_currentScheduleIndex != index) {
        m_currentScheduleIndex = index;
        emit currentScheduleIndexChanged();
        emit scheduleChanged();
    }
}


QVariantList SchedulesDisplayController::getDayItems(int scheduleIndex, int dayIndex) const {
    if (scheduleIndex < 0 || scheduleIndex >= static_cast<int>(m_schedules.size()))
        return {};

    if (dayIndex < 0 || dayIndex >= static_cast<int>(m_schedules[scheduleIndex].week.size()))
        return QVariantList();

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

QString SchedulesDisplayController::getDayName(int dayIndex) const {
    const QString dayNames[] = {
        "ראשון", "שני", "שלישי", "רביעי", "חמישי", "שישי", "שבת"
    };

    if (dayIndex >= 0 && dayIndex < 7)
        return dayNames[dayIndex];
    return "";
}

int SchedulesDisplayController::getScheduleCount() const {
    return static_cast<int>(m_schedules.size());
}

void SchedulesDisplayController::saveCurrentSchedule(const QString &path) {
    if (m_currentScheduleIndex >= 0 && m_currentScheduleIndex < static_cast<int>(m_schedules.size())) {
        Model model;
        // יש להמיר InformativeSchedule ל-Schedule לפני שמירה
        Schedule schedule;
        // TODO: המרה מ-InformativeSchedule ל-Schedule

        model.executeOperation(ModelOperation::SAVE_SCHEDULE, &schedule, path.toStdString());
    }
}

void SchedulesDisplayController::printCurrentSchedule() {
    if (m_currentScheduleIndex >= 0 && m_currentScheduleIndex < static_cast<int>(m_schedules.size())) {
        Model model;
        // יש להמיר InformativeSchedule ל-Schedule לפני הדפסה
        Schedule schedule;
        // TODO: המרה מ-InformativeSchedule ל-Schedule

        model.executeOperation(ModelOperation::PRINT_SCHEDULE, &schedule);
    }
}

void SchedulesDisplayController::goBack() {
    // חזרה למסך בחירת הקורסים
    emit navigateBack();
}