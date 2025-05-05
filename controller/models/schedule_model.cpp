#include "schedule_model.h"

ScheduleModel::ScheduleModel(QObject* parent)
        : QAbstractTableModel(parent)
{
    // Constructor implementation
}

void ScheduleModel::populateSchedulesData(const vector<InformativeSchedule>& schedules)
{
    beginResetModel();

    m_schedules.clear();

    // Convert InformativeSchedule to InformativeScheduleM
    for (const auto& schedule : schedules) {
        vector<ScheduleDayM> weekM;

        for (const auto& day : schedule.week) {
            vector<ScheduleItemM> itemsM;

            for (const auto& item : day.day_items) {
                itemsM.emplace_back(
                        QString::fromStdString(item.courseName),
                        QString::fromStdString(item.raw_id),
                        QString::fromStdString(item.type),
                        QString::fromStdString(item.start),
                        QString::fromStdString(item.end),
                        QString::fromStdString(item.building),
                        QString::fromStdString(item.room)
                );
            }

            weekM.emplace_back(
                    QString::fromStdString(day.day),
                    itemsM
            );
        }

        m_schedules.emplace_back(schedule.index, weekM);
    }

    endResetModel();
}

// QAbstractTableModel required overrides
int ScheduleModel::rowCount(const QModelIndex& parent) const
{
    if (parent.isValid())
        return 0;

    return m_schedules.size();
}

int ScheduleModel::columnCount(const QModelIndex& parent) const
{
    if (parent.isValid())
        return 0;

    // Return number of days in a week (assuming 7 days)
    return 7;
}

QVariant ScheduleModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (role != Qt::DisplayRole)
        return {};

    if (orientation == Qt::Horizontal) {
        // Days of the week
        static const QStringList days = {"Sunday", "Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday"};
        if (section < days.size())
            return days[section];
    } else if (orientation == Qt::Vertical) {
        // Schedule index
        return tr("Schedule %1").arg(m_schedules[section].index);
    }

    return {};
}

// Additional methods could be added here for more functionality
Qt::ItemFlags ScheduleModel::flags(const QModelIndex& index) const
{
    if (!index.isValid())
        return Qt::NoItemFlags;

    return QAbstractTableModel::flags(index) | Qt::ItemIsEnabled | Qt::ItemIsSelectable;
}

QVariant ScheduleModel::data(const QModelIndex& index, int role) const
{
    if (!index.isValid() || index.row() >= static_cast<int>(m_schedules.size()) || index.column() >= 7)
        return {};

    const auto& schedule = m_schedules[index.row()];

    if (role == Qt::DisplayRole) {
        if (index.column() < static_cast<int>(schedule.week.size())) {
            const auto& day = schedule.week[index.column()];

            // Return a summary of the day's schedule items
            if (day.day_items.empty())
                return tr("No classes");

            return tr("%1 classes").arg(day.day_items.size());
        }
    }
    else if (role == DayItemsRole) {
        if (index.column() < static_cast<int>(schedule.week.size())) {
            const auto& day = schedule.week[index.column()];

            // Return the detailed items as a QVariantList
            QVariantList items;
            for (const auto& item : day.day_items) {
                QVariantMap itemMap;
                itemMap["courseName"] = item.courseName;
                itemMap["raw_id"] = item.raw_id;
                itemMap["type"] = item.type;
                itemMap["start"] = item.start;
                itemMap["end"] = item.end;
                itemMap["building"] = item.building;
                itemMap["room"] = item.room;
                items.append(itemMap);
            }
            return items;
        }
    }

    return {};
}

// Add this method to expose roles to QML
QHash<int, QByteArray> ScheduleModel::roleNames() const
{
    QHash<int, QByteArray> roles = QAbstractTableModel::roleNames();
    roles[DayItemsRole] = "dayItems";
    return roles;
}