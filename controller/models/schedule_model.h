#ifndef SCHEDULE_MODEL_H
#define SCHEDULE_MODEL_H

#include "main_include.h"

#include <QAbstractTableModel>
#include <utility>
#include <vector>
#include <string>
#include <QStringList>

struct ScheduleItemM {
    QString courseName;
    QString raw_id;
    QString type;
    QString start;
    QString end;
    QString building;
    QString room;

    ScheduleItemM(QString courseName, QString raw_id, QString type, QString start, QString end, QString building,
                  QString room)
            : courseName(std::move(courseName)), raw_id(std::move(raw_id)), type(std::move(type)), start(std::move(start)),
              end(std::move(end)), building(std::move(building)), room(std::move(room)) {}
};

struct ScheduleDayM {
    QString day;
    std::vector<ScheduleItemM> day_items;

    ScheduleDayM(QString day, std::vector<ScheduleItemM> day_items)
            : day(std::move(day)), day_items(std::move(day_items)) {}
};

struct InformativeScheduleM {
    int index;
    std::vector<ScheduleDayM> week;

    InformativeScheduleM(int index, std::vector<ScheduleDayM> week)
            : index(index), week(std::move(week)) {}
};

// Forward declarations for the original data structures
struct ScheduleItem;
struct ScheduleDay;
struct InformativeSchedule;

// Qt table model for schedules
class ScheduleModel : public QAbstractTableModel {
Q_OBJECT

public:
    explicit ScheduleModel(QObject* parent = nullptr);

    // Method to populate the model with data
    void populateSchedulesData(const std::vector<InformativeSchedule>& schedules);

    enum Roles {
        DayItemsRole = Qt::UserRole + 1
    };

    // QAbstractTableModel required overrides
    int rowCount(const QModelIndex& parent = QModelIndex()) const override;
    int columnCount(const QModelIndex& parent = QModelIndex()) const override;
    QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;
    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;
    Qt::ItemFlags flags(const QModelIndex& index) const override;
    QHash<int, QByteArray> roleNames() const override;

private:
    std::vector<InformativeScheduleM> m_schedules;
};

#endif // SCHEDULE_MODEL_H