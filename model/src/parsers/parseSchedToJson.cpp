#include "parsers/parseSchedToJson.h"
#include "parsers/ScheduleEnrichment.h"
#include "logs/logger.h"

bool exportSchedulesToJson(const vector<Schedule>& schedules, const string& outputPath, const vector<Course>& courses) {
    ofstream outFile(outputPath);
    if (!outFile.is_open()) {
        ostringstream message;
        message << "Cannot open file: " << outputPath << ", aborting...";
        Logger::get().logError(message.str());
        return false;
    }

    vector<Schedule> validSchedules = filterValidSchedules(schedules, courses);

    if (validSchedules.empty()) {
        Logger::get().logError("There are no valid schedules, aborting...");
        return false;
    }

    outFile << "[";

    for (size_t i = 0; i < validSchedules.size(); ++i) {
        const auto& schedule = validSchedules[i];
        outFile << "{\"schedule_number\":" << (i + 1) << ",\"schedule\":[";

        // Build the dayMap for the current schedule
        auto dayMap = buildDayMapForSchedule(schedule, courses, exportType::JSON);

        bool dayWritten = false;
        for (int day = 0; day < 7; ++day) {
            if (dayMap.count(day)) {
                if (dayWritten) outFile << ",";
                dayWritten = true;

                // Write the day's schedule to JSON
                writeDayScheduleToJson(outFile, dayMap, day);
            }
        }

        outFile << "]}";
        if (i + 1 < validSchedules.size()) outFile << ",";
    }

    outFile << "]";
    outFile.close();

    return true;
}

void writeDayScheduleToJson(ostream& outFile, const unordered_map<int, vector<ScheduleItem>>& dayMap, int day) {
    if (dayMap.count(day)) {
        outFile << R"({"day":")" << dayToString(day) << R"(","schedule_items":[)";

        // Create a non-const copy of the vector
        vector<ScheduleItem> items = dayMap.at(day);

        // Sort the copied vector
        sort(items.begin(), items.end(), [](const ScheduleItem& a, const ScheduleItem& b) {
            return a.start < b.start;
        });

        // Write the sorted schedule items
        for (size_t j = 0; j < items.size(); ++j) {
            outFile << ScheduleItemToJson(items[j]);
            if (j + 1 < items.size()) outFile << ",";
        }

        outFile << "]}";
    }
}

string ScheduleItemToJson(const ScheduleItem& s) {
    stringstream ss;
    ss << "{\"course_name\":" << s.courseName
       << R"(,"course_id":")" << s.raw_id << "\""
       << R"(,"item_type":")" << s.type << "\""
       << R"(,"start":")" << s.start << "\""
       << R"(,"end":")" << s.end << "\""
       << R"(,"building":")" << s.building << "\""
       << R"(,"room":")" << s.room << "\"}";
    return ss.str();
}