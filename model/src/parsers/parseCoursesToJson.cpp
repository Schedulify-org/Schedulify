#include "parseCoursesToJson.h"

bool exportCoursesToJson(const vector<Course>& courses, const string& outputPath) {
    ofstream outFile(outputPath);

    if (!outFile.is_open()) {
        ostringstream message;
        message << "Cannot open file: " << outputPath << ", aborting...";
        Logger::get().logError(message.str());
        return false;
    }

    vector<Course> validCourses = filterValidCourses(courses);

    if (validCourses.empty()) {
        Logger::get().logError("There are no valid courses, aborting...");
        return false;
    }

    outFile << "[";

    for (size_t i = 0; i < validCourses.size(); ++i) {
        const auto& course = validCourses[i];
        outFile << "{\"course_id\": " << "\"" + course.raw_id + "\""
                << ",\"course_name\": " << "\"" + course.name + "\""
                << ",\"teacher\": " << "\"" + course.teacher + "\"";

        outFile << "}";
        if (i + 1 < validCourses.size()) outFile << ",";
    }

    outFile << "]";
    outFile.close();

    return true;
}

vector<Course> filterValidCourses(const vector<Course>& courses) {
    vector<Course> validCourses;

    for (const auto& course : courses) {
        if (course.raw_id.empty() || course.name.empty() || course.teacher.empty()) continue;

        if (course.Lectures.empty() && course.Tirgulim.empty() && course.labs.empty()) continue;

        validCourses.push_back(course);
    }

    return validCourses;
}
