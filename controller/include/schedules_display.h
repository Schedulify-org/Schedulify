#ifndef SCHEDULES_DISPLAY_H
#define SCHEDULES_DISPLAY_H

#include "controller_manager.h"
#include "controller/models/schedule_model.h"

class SchedulesDisplayController : public ControllerManager {
    Q_OBJECT
    Q_PROPERTY(ScheduleModel* scheduleModel READ scheduleModel CONSTANT)

public:
    explicit SchedulesDisplayController(QObject *parent = nullptr);
    ~SchedulesDisplayController() override = default;

    void loadScheduleData(const std::vector<InformativeSchedule>& schedules);

    [[nodiscard]] ScheduleModel* scheduleModel() const { return m_scheduleModel; }

signals:
    void scheduleDataLoaded();

private:
    ScheduleModel* m_scheduleModel = nullptr;
};

#endif //SCHEDULES_DISPLAY_H
