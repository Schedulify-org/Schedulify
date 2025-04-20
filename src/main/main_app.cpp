#include "main/main_app.h"
#include "logs/logger.h"

int main_app(const string& inputPath, const string& modifiedOutputPath ,const string& userInput) {

    Logger::get().initialize();

    Logger::get().logInfo("Input parsing started");

    vector<Course> courses = parseCourseDB(inputPath, userInput);

    if (courses.empty()) {
        Logger::get().logError("Error while parsing input data. aborting process");
        return 1;
    }

    Logger::get().logInfo("initiate schedules builder");

    ScheduleBuilder builder;
    vector<Schedule> schedules = builder.build(courses);

    if (schedules.empty()) {
        Logger::get().logError("received empty result from algorithm, aborting process");
        return 1;
    }

    Logger::get().logInfo("initiate output generation");

    bool success = exportSchedulesToJson(schedules, modifiedOutputPath, courses);

    if (success) {
        ostringstream message;
        message << "output finish, can be seen in " << modifiedOutputPath;
        Logger::get().logInfo(message.str());
    }

    return 0;
}

