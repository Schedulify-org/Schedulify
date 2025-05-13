#include "parsers/parseToCsv.h"

int getHourFromTimeString(const string& timeStr) {
    try {
        return stoi(timeStr.substr(0, 2));
    } catch (const exception& e) {
        Logger::get().logError("Error parsing time string: " + timeStr + " - " + e.what());
        return 0;
    }
}

bool saveScheduleToCsv(const string& filePath, const InformativeSchedule& schedule) {
    ofstream csvFile(filePath);
    if (!csvFile.is_open()) {
        Logger::get().logError("Error opening file: " + filePath);
        return false;
    }

    vector<string> days;
    for (const auto& day : schedule.week) {
        days.push_back(day.day);
    }

    const int minHour = 8;
    const int maxHour = 20;

    // Create a map to store all schedule data
    map<pair<int, string>, string> scheduleData;

    // Fill in the schedule data
    for (const auto& day : schedule.week) {
        for (const auto& item : day.day_items) {
            int startHour;
            int endHour;

            try {
                startHour = getHourFromTimeString(item.start);
                endHour = getHourFromTimeString(item.end);
            } catch (const exception& e) {
                Logger::get().logError("Failed to parse time for item: " + item.courseName + " - " + e.what());
                continue;
            }

            for (int hour = startHour; hour < endHour; hour++) {
                // Create cell content with formatting like in the picture
                stringstream cellContent;
                cellContent << item.courseName << "\n"
                            << item.raw_id << " - " << item.type << "\n"
                            << item.start << " - " << item.end << "\n"
                            << "Building: " << item.building << ", Room: " << item.room;

                // Store in map with (hour, day) as key
                scheduleData[{hour, day.day}] = cellContent.str();
            }
        }
    }

    csvFile << "Hour/Day";
    for (const auto& day : days) {
        csvFile << "," << day;
    }
    csvFile << endl;

    // Write data rows
    for (int hour = minHour; hour < maxHour; hour++) {
        stringstream timeStr;
        timeStr << setw(2) << setfill('0') << hour << ":00-"
                << setw(2) << setfill('0') << (hour + 1) << ":00";
        csvFile << timeStr.str();

        for (const auto& day : days) {
            csvFile << ",";

            auto key = make_pair(hour, day);
            if (scheduleData.find(key) != scheduleData.end()) {
                string content = scheduleData[key];
                size_t pos = 0;
                while ((pos = content.find('\"', pos)) != string::npos) {
                    content.replace(pos, 1, "\"\"");
                    pos += 2;
                }
                csvFile << "\"" << content << "\"";
            }
        }

        csvFile << endl;
    }

    csvFile.close();
    Logger::get().logInfo("Schedule successfully saved to " + filePath);
    return true;
}