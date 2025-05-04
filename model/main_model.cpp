#include "main/main_model.h"
#include "parsers/parseSchedToJson.h"
#include <filesystem>
#include "logs/logger.h"

// this is the main model to run schedulify 2.0

void testSched(const vector<Course>& lastGeneratedCourses, const vector<Schedule>& lastGeneratedSchedules) {
    namespace fs = std::filesystem;
    fs::path main_path = fs::current_path().parent_path();
    string modifiedOutputPath = main_path.string() + OUTPUTPATH + "new.json";
    bool success = exportSchedulesToJson(lastGeneratedSchedules, modifiedOutputPath, lastGeneratedCourses);
}

vector<Course> Model::generateCourses(const string& path) {
    vector<Course> courses = parseCourseDB(path);

    if (courses.empty()) {
        Logger::get().logError("Error while parsing input data. aborting process");
    }

    return courses;
}

vector<Schedule> Model::generateSchedules(const vector<Course>& userInput) {
    if (userInput.empty() || userInput.size() > 7) {
        Logger::get().logError("invalid amount of courses, aborting...");
        return {};
    }

    ScheduleBuilder builder;
    vector<Schedule> schedules = builder.build(userInput);

    if (schedules.empty()) {
        Logger::get().logError("unable to generate schedules, aborting process");
    }

    return schedules;
}

void Model::saveSchedule(const Schedule& schedule, const string& path) {

}

void Model::printSchedule(const Schedule& schedule) {

}

void* Model::executeOperation(ModelOperation operation, const void* data, const string& path) {
    switch (operation) {
        case ModelOperation::GENERATE_COURSES:
            if (!path.empty()) {
                lastGeneratedCourses = generateCourses(path);
                return &lastGeneratedCourses;
            }
            // Handle error case where path is null
            break;


        case ModelOperation::GENERATE_SCHEDULES:
            if (data) {
                const auto* courses = static_cast<const vector<Course>*>(data);
                lastGeneratedSchedules = generateSchedules(*courses);
                testSched(lastGeneratedCourses, lastGeneratedSchedules);
                return &lastGeneratedSchedules;
            }
            // Handle error case where data is null
            break;

        case ModelOperation::SAVE_SCHEDULE:
            if (data && !path.empty()) {
                const auto* schedule = static_cast<const Schedule*>(data);
                saveSchedule(*schedule, path);
            }
            // Handle error case where data or path is null
            break;

        case ModelOperation::PRINT_SCHEDULE:
            if (data) {
                const auto* schedule = static_cast<const Schedule*>(data);
                printSchedule(*schedule);
            }
            // Handle error case where data is null
            break;
    }
    return nullptr;
}
