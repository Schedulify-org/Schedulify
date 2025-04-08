#include "main/main_app.h"

int main_app(const string& inputPath, const string& modifiedOutputPath ,const string& userInput) {

    vector<Course> courses = parseCourseDB(inputPath, userInput);

    ScheduleBuilder builder;
    vector<Schedule> schedules = builder.build(courses);

    exportSchedulesToText(schedules, modifiedOutputPath, courses);

    return 0;
}

