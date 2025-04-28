#include <filesystem>
#include "main/main_model.h"
#include "logs/logger.h"

int app_main(const string& action_selected) {
    namespace fs = std::filesystem;

    fs::path main_path = fs::current_path().parent_path();

    Logger::get().initialize();

    Logger::get().logInfo("directory: " + main_path.string());

    map<string, ICommand*> commands = initiate_main_menu();

    if (!keyExistsInMap(commands, action_selected)) {
        Logger::get().logInfo("Bad request, " + action_selected + " is not a valid commend");
        return 1;
    }

    try {
        bool status = commands[action_selected]->execute(main_path.string());

        if (status) Logger::get().logInfo(action_selected + " finished successfully");
        else Logger::get().logInfo("Error, unable to complete " + action_selected);
    } catch (...) {
        Logger::get().logInfo("Error, unable to complete " + action_selected);
        return 1;
    }

    return 0;
}

map<string, ICommand*> initiate_main_menu() {
    map<string, ICommand*> commands;

    ICommand* GenerateCourses = new GenerateCourseFile();
    commands["COURSES"] = GenerateCourses;

    ICommand* GenerateSchedules = new GenerateSchedFile();
    commands["SCHEDULES"] = GenerateSchedules;

    return commands;
}

bool keyExistsInMap(const map<string, ICommand *>& commands, const std::string& key) {
    for (const auto& pair : commands) {
        if (pair.first == key) {
            return true; // Key found
        }
    }
    return false; // Key not found
}

bool GenerateCourseFile::execute(string main_path) {

    string inputPath = main_path + COURSEDBINPUT;;

    string courseOutput = main_path + OUTPUTCOURSEPATH;

    vector<Course> courses = parseCourseDB(inputPath);

    if (courses.empty()) {
        Logger::get().logError("Error while parsing input data. aborting process");
        return false;
    }

    bool cSuccess = exportCoursesToJson(courses, courseOutput);

    if (cSuccess) {
        ostringstream message;
        message << "output finish, can be seen in " << courseOutput;
        Logger::get().logInfo(message.str());
    } else {
        Logger::get().logError("Error while parsing input data. aborting process");
        return false;
    }

    return true;
}

bool GenerateSchedFile::execute(string main_path) {

    string modifiedOutputPath = main_path + OUTPUTPATH;
    string inputPath = main_path + COURSEDBINPUT;
    string userInput = main_path + USERINPUT;

    vector<Course> courses = parseCourseDB(inputPath);

    vector<Course> filteredCourses = extractUserChoice(userInput, courses);

    if (filteredCourses.empty()) {
        Logger::get().logError("no valid choices were found, aborting...");
        return false;
    }

    ScheduleBuilder builder;
    vector<Schedule> schedules = builder.build(filteredCourses);

    if (schedules.empty()) {
        Logger::get().logError("unable to generate schedules, aborting process");
        return false;
    }

    bool success = exportSchedulesToJson(schedules, modifiedOutputPath, filteredCourses);

    if (success) {
        ostringstream message;
        message << "output finish, can be seen in " << modifiedOutputPath;
        Logger::get().logInfo(message.str());
    } else {
        Logger::get().logError("unable to generate schedules, aborting process");
        return false;
    }

    return true;
}