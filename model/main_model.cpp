#include "main_model.h"

std::string getFileExtension(const std::string& filename) {
    size_t dot = filename.find_last_of(".");
    if (dot == std::string::npos) {
        return "";
    }
    std::string ext = filename.substr(dot + 1);
    std::transform(ext.begin(), ext.end(), ext.begin(), ::tolower);
    return ext;
}

vector<Course> Model::generateCourses(const string& path) {
    vector<Course> courses;

    // Determine file type and use appropriate parser
    std::string extension = getFileExtension(path);

    if (extension == "xlsx") {
        Logger::get().logInfo("Parsing Excel file: " + path);
        ExcelCourseParser excelParser;
        courses = excelParser.parseExcelFile(path);
    }
    else if (extension == "txt") {
        Logger::get().logInfo("Parsing text file: " + path);
        courses = parseCourseDB(path);
    }
    else {
        Logger::get().logError("Unsupported file format: " + extension + ". Supported formats: .txt, .xlsx");
        return courses; // Return empty vector
    }

    if (courses.empty()) {
        Logger::get().logError("Error while parsing input data from file: " + path + ". No courses found.");
    } else {
        Logger::get().logInfo("Successfully parsed " + std::to_string(courses.size()) + " courses from " + path);
    }

    return courses;
}

vector<string> Model::validateCourses(const vector<Course>& courses) {
    if (courses.empty()) {
        Logger::get().logError("No courses were found");
        return {};
    }
    return validate_courses(courses);
}

vector<InformativeSchedule> Model::generateSchedules(const vector<Course>& userInput) {
    if (userInput.empty() || userInput.size() > 8) {
        Logger::get().logError("invalid amount of courses, aborting...");
        return {};
    }

    ScheduleBuilder builder;
    vector<InformativeSchedule> schedules = builder.build(userInput);

    if (schedules.empty()) {
        Logger::get().logError("unable to generate schedules, aborting process");
    }

    return schedules;
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
                return nullptr;
            }

        case ModelOperation::VALIDATE_COURSES:
            if (data) {
                const auto* courses = static_cast<const vector<Course>*>(data);
                courseFileErrors = validateCourses(*courses);
                return &courseFileErrors;
            } else {
                Logger::get().logError("No courses were found, aborting...");
                return nullptr;
            }

        case ModelOperation::GENERATE_SCHEDULES:
            if (data) {
                const auto* courses = static_cast<const vector<Course>*>(data);
                lastGeneratedSchedules = generateSchedules(*courses);
                return &lastGeneratedSchedules;
            } else {
                Logger::get().logError("unable to generate schedules, aborting...");
                return nullptr;
            }

        case ModelOperation::SAVE_SCHEDULE:
            if (data && !path.empty()) {
                const auto* schedule = static_cast<const InformativeSchedule*>(data);
                saveSchedule(*schedule, path);
            } else {
                Logger::get().logError("unable to save schedule, aborting...");
            }
            break;

        case ModelOperation::PRINT_SCHEDULE:
            if (data) {
                const auto* schedule = static_cast<const InformativeSchedule*>(data);
                printSchedule(*schedule);
            } else {
                Logger::get().logError("unable to print schedule, aborting...");
            }
            break;
    }
    return nullptr;
}