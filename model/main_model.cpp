#include "main_model.h"

// this is the main model to run Schedulify 2.0

vector<Course> Model::generateCourses(const string& path) {
    vector<Course> courses = parseCourseDB(path);

    if (courses.empty()) {
        Logger::get().logError("Error while parsing input data. aborting process");
    }

    return courses;
}

vector<InformativeSchedule> Model::generateSchedules(const vector<Course>& userInput) {
    if (userInput.empty() || userInput.size() > 7) {
        Logger::get().logError("invalid amount of courses, aborting...");
        return {};
    }

    ScheduleBuilder builder;
    vector<Schedule> schedules = builder.build(userInput);

    if (schedules.empty()) {
        Logger::get().logError("unable to generate schedules, aborting process");
    }

    vector<InformativeSchedule> informativeSchedules = exportSchedulesToObjects(schedules, userInput);

    if (informativeSchedules.empty()) {
        Logger::get().logError("unable to generate schedules, aborting process");
    }

    return informativeSchedules;
}

void Model::saveSchedule(const InformativeSchedule& infoSchedule, const string& path) {
    bool status = saveScheduleToCsv(path, infoSchedule);
    string message = status ? "Schedule saved to CSV: " + path : "An error has accrued, unable to save schedule as csv";
    Logger::get().logInfo(message);
}

void Model::printSchedule(const InformativeSchedule& infoSchedule) {
    bool status = printSelectedSchedule(infoSchedule);
    string message = status ? "Schedule sent to printer" : "An error has accrued, unable to print schedule";
    Logger::get().logInfo(message);
}

void* Model::executeOperation(ModelOperation operation, const void* data, const string& path) {
    switch (operation) {
        case ModelOperation::GENERATE_COURSES:
            if (!path.empty()) {
                lastGeneratedCourses = generateCourses(path);
                return &lastGeneratedCourses;
            } else {
                Logger::get().logError("File not found, aborting...");
                return {};
            }

        case ModelOperation::GENERATE_SCHEDULES:
            if (data) {
                const auto* courses = static_cast<const vector<Course>*>(data);
                lastGeneratedSchedules = generateSchedules(*courses);
                return &lastGeneratedSchedules;
            } else {
                Logger::get().logError("unable to generate schedules, aborting...");
                return {};
            }

        case ModelOperation::SAVE_SCHEDULE:
            if (data && !path.empty()) {
                const auto* schedule = static_cast<const InformativeSchedule*>(data);
                saveSchedule(*schedule, path);
            } else {
                Logger::get().logError("unable to generate schedules, aborting...");
            }
            // Handle error case where data or path is null
            break;

        case ModelOperation::PRINT_SCHEDULE:
            if (data) {
                const auto* schedule = static_cast<const InformativeSchedule*>(data);
                printSchedule(*schedule);
            }
            // Handle error case where data is null
            break;
    }
    return nullptr;
}
