#ifndef SCHEDULE_MODEL_H
#define SCHEDULE_MODEL_H

#include <QObject>
#include <QVariant>
#include "model_interfaces.h"

class ScheduleModel : public QObject {
Q_OBJECT
    Q_PROPERTY(int currentScheduleIndex READ currentScheduleIndex
                       WRITE setCurrentScheduleIndex NOTIFY currentScheduleIndexChanged)
    Q_PROPERTY(int scheduleCount READ scheduleCount NOTIFY scheduleCountChanged)
    Q_PROPERTY(bool canGoNext READ canGoNext NOTIFY currentScheduleIndexChanged)
    Q_PROPERTY(bool canGoPrevious READ canGoPrevious NOTIFY currentScheduleIndexChanged)

public:
    explicit ScheduleModel(QObject *parent = nullptr);
    ~ScheduleModel() override = default;

    // Schedule management
    void loadSchedules(const std::vector<InformativeSchedule>& schedules);

    // Properties
    int currentScheduleIndex() const { return m_currentScheduleIndex; }
    Q_INVOKABLE void setCurrentScheduleIndex(int index);
    int scheduleCount() const { return static_cast<int>(m_schedules.size()); }

    // QML accessible methods
    Q_INVOKABLE QVariantList getDayItems(int scheduleIndex, int dayIndex) const;
    Q_INVOKABLE QVariantList getCurrentDayItems(int dayIndex) const;
    Q_INVOKABLE void nextSchedule();
    Q_INVOKABLE void previousSchedule();
    Q_INVOKABLE bool canGoNext() const;
    Q_INVOKABLE bool canGoPrevious() const;

signals:
    void currentScheduleIndexChanged();
    void scheduleCountChanged();
    void scheduleDataChanged();

private:
    std::vector<InformativeSchedule> m_schedules;
    int m_currentScheduleIndex;
};

#endif // SCHEDULE_MODEL_H