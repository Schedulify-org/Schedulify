#ifndef SCHEDULES_DISPLAY_H
#define SCHEDULES_DISPLAY_H

#include "controller_manager.h"
#include "controller/models/schedule_model.h"
#include <QObject>
#include <QVariant>
#include <QAbstractListModel>
#include "main_include.h"

class ScheduleListModel : public QAbstractListModel {
    Q_OBJECT
public:
    enum Roles {
        IndexRole = Qt::UserRole + 1,
        DaysRole
    };

    explicit ScheduleListModel(QObject *parent = nullptr);
    void setSchedules(const vector<InformativeSchedule>& schedules);

    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    QHash<int, QByteArray> roleNames() const override;

private:
    vector<InformativeSchedule> m_schedules;
};

class SchedulesDisplayController : public ControllerManager {
    Q_OBJECT
    Q_PROPERTY(ScheduleListModel* schedulesModel READ schedulesModel CONSTANT)
    Q_PROPERTY(int currentScheduleIndex READ currentScheduleIndex WRITE setCurrentScheduleIndex NOTIFY currentScheduleIndexChanged)

public:
    explicit SchedulesDisplayController(QObject *parent = nullptr);
    ~SchedulesDisplayController();

    void loadScheduleData(const vector<InformativeSchedule>& schedules);

    ScheduleListModel* schedulesModel() const;
    int currentScheduleIndex() const;
    Q_INVOKABLE void setCurrentScheduleIndex(int index);

    Q_INVOKABLE QVariantList getDayItems(int scheduleIndex, int dayIndex) const;
    Q_INVOKABLE QString getDayName(int dayIndex) const;
    Q_INVOKABLE int getScheduleCount() const;
    Q_INVOKABLE void saveCurrentSchedule(const QString& path);
    Q_INVOKABLE void printCurrentSchedule();
    Q_INVOKABLE void goBack();

    signals:
        void currentScheduleIndexChanged();
        void scheduleChanged();

private:
    ScheduleListModel *m_schedulesModel;
    vector<InformativeSchedule> m_schedules;
    int m_currentScheduleIndex;
};

#endif // SCHEDULESDISPLAYCONTROLLER_H