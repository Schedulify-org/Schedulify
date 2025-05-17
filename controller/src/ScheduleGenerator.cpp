#include "ScheduleGenerator.h"

ScheduleGenerator::ScheduleGenerator(IModel* modelConn, const std::vector<Course>& courses, QObject* parent)
        : QObject(parent),
          modelConnection(modelConn),
          selectedCourses(courses) {}

void ScheduleGenerator::generateSchedules() {
    auto* schedulePtr = static_cast<std::vector<InformativeSchedule>*>
    (modelConnection->executeOperation(ModelOperation::GENERATE_SCHEDULES, &selectedCourses, ""));
    emit schedulesGenerated(schedulePtr);
}