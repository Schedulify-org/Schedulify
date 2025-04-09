#include "main/main_app.h"
#include "logs/logger.h"

int main_app(const string& inputPath, const string& modifiedOutputPath ,const string& userInput) {

    Logger::get().initialize();

    Logger::get().logInfo("Input parsing started");

    vector<Course> courses = parseCourseDB(inputPath, userInput);

    Logger::get().logInfo("initiate schedules builder");

    ScheduleBuilder builder;
    vector<Schedule> schedules = builder.build(courses);

    Logger::get().logInfo("schedules build successfully, generating output");

    exportSchedulesToText(schedules, modifiedOutputPath, courses);

    ostringstream message;
    message << "output finish, can be seen in " << modifiedOutputPath;
    Logger::get().logInfo(message.str());

    return 0;
}

