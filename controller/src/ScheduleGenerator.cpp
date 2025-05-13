#include "ScheduleGenerator.h"

ScheduleGenerator::ScheduleGenerator(IModel* modelConn,
                                     const std::vector<Course>& courses,
                                     QObject* parent)
        : QObject(parent),
          modelConnection(modelConn),
          selectedCourses(courses),
          abortRequested(false) {}

void ScheduleGenerator::generateSchedules() {
    // Check if abort was requested before starting
    if (abortRequested) {
        emit schedulesGenerated(nullptr);
        return;
    }

    // Execute the operation in the background thread
    auto* schedulePtr = static_cast<std::vector<InformativeSchedule>*>
    (modelConnection->executeOperation(ModelOperation::GENERATE_SCHEDULES, &selectedCourses, ""));

    // If abort was requested during execution, clean up
    if (abortRequested) {
        delete schedulePtr;
        emit schedulesGenerated(nullptr);
    } else {
        emit schedulesGenerated(schedulePtr);
    }
}

void ScheduleGenerator::abort() {
    abortRequested = true;
}