#include <QString>
#include "main/main_model.h"
#include "logs/logger.h"
#include "parsers/printSchedule.h"

// this is the main model to run Schedulify 2.0

vector<Course> Model::generateCourses(const string& path) {
    vector<Course> courses = parseCourseDB(path);

    if (courses.empty()) {
        Logger::get().logError("Error while parsing input data. aborting process");
    }

    return courses;
}

vector<InformativeSchedule> Model::generateSchedules(const vector<Course>& userInput, const vector<Course>& allCourses) {
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

    printInformativeSchedules(informativeSchedules);

    if (informativeSchedules.empty()) {
        Logger::get().logError("unable to generate schedules, aborting process");
    }

    return informativeSchedules;
}

void Model::saveSchedule(const Schedule& schedule, const string& path) {
    // Convert Schedule to InformativeSchedule for PDF export
    vector<Course> courses; // Ideally, this would come from somewhere else
    vector<Schedule> singleSchedule = {schedule};
    vector<InformativeSchedule> informativeSchedules = exportSchedulesToObjects(singleSchedule, courses);

    if (!informativeSchedules.empty()) {
        // Use the saveToPDF function from printSchedule.h
        saveToPDF(informativeSchedules[0], QString::fromStdString(path));
        Logger::get().logInfo("Schedule saved to PDF: " + path);
    } else {
        Logger::get().logError("Failed to convert schedule for PDF export");
    }
}

void Model::printSchedule(const Schedule& schedule) {
    // Convert Schedule to InformativeSchedule for printing
    vector<Course> courses; // Ideally, this would come from somewhere else
    vector<Schedule> singleSchedule = {schedule};
    vector<InformativeSchedule> informativeSchedules = exportSchedulesToObjects(singleSchedule, courses);

    if (!informativeSchedules.empty()) {
        // Use the printSchedule function from printSchedule.h
        ::printSchedule(informativeSchedules[0]);
        Logger::get().logInfo("Schedule sent to printer");
    } else {
        Logger::get().logError("Failed to convert schedule for printing");
    }
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
                lastGeneratedSchedules = generateSchedules(*courses, lastGeneratedCourses);
                return &lastGeneratedSchedules;
            }
            Logger::get().logError("unable to generate schedules, aborting process");
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
