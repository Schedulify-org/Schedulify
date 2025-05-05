#include "schedules_display.h"

SchedulesDisplayController::SchedulesDisplayController(QObject *parent)
        : ControllerManager(parent)
        , m_scheduleModel(new ScheduleModel(this))
{
}

// Add this function to load schedule data
void SchedulesDisplayController::loadScheduleData(const std::vector<InformativeSchedule>& schedules)
{
    // Call the populate method on the model
    m_scheduleModel->populateSchedulesData(schedules);

    // Emit signal that data is loaded (optional)
    emit scheduleDataLoaded();
}