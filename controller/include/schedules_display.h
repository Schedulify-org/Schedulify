#ifndef SCHEDULES_DISPLAY_H
#define SCHEDULES_DISPLAY_H

#include "controller_manager.h"
#include <QObject>
#include <QVariant>
#include "main_include.h"

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
    Q_INVOKABLE void saveCurrentSchedule(const QString& path) const;
    Q_INVOKABLE void printCurrentSchedule() const;
    Q_INVOKABLE void goBack() override;

    signals:
        void currentScheduleIndexChanged();
        void scheduleChanged();

private:
    vector<InformativeSchedule> m_schedules;
    int m_currentScheduleIndex;
};

#endif // SCHEDULES_DISPLAY_H