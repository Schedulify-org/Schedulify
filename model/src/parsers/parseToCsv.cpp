#include "parsers/parseToCsv.h"

// Helper function to convert time string to hour integer
int getHourFromTimeString(const std::string& timeStr) {
    // Assuming format is "HH:MM" or "HH:MM:SS"
    return std::stoi(timeStr.substr(0, 2));
}

void saveScheduleToCsv(const std::string& filePath, const InformativeSchedule& schedule) {
    // Open output file
    std::ofstream csvFile(filePath);
    if (!csvFile.is_open()) {
        std::cerr << "Error opening file: " << filePath << std::endl;
        return;
    }

    // Collect all days
    std::vector<std::string> days;
    for (const auto& day : schedule.week) {
        days.push_back(day.day);
    }

    // Find min and max hours in the schedule
    int minHour = 24;
    int maxHour = 0;

    for (const auto& day : schedule.week) {
        for (const auto& item : day.day_items) {
            int startHour = getHourFromTimeString(item.start);
            int endHour = getHourFromTimeString(item.end);

            minHour = std::min(minHour, startHour);
            maxHour = std::max(maxHour, endHour);
        }
    }

    // Create a map to store all schedule data
    std::map<std::pair<int, std::string>, std::string> scheduleData;

    // Fill in the schedule data
    for (const auto& day : schedule.week) {
        for (const auto& item : day.day_items) {
            int startHour = getHourFromTimeString(item.start);
            int endHour = getHourFromTimeString(item.end);

            for (int hour = startHour; hour < endHour; hour++) {
                // Create cell content
                std::stringstream cellContent;
                cellContent << item.courseName << " (" << item.raw_id << ") "
                            << item.type << " " << item.building << " " << item.room;

                // Store in map with (hour, day) as key
                scheduleData[{hour, day.day}] = cellContent.str();
            }
        }
    }

    // Write header row
    csvFile << "Time";
    for (const auto& day : days) {
        csvFile << "," << day;
    }
    csvFile << std::endl;

    // Write data rows
    for (int hour = minHour; hour < maxHour; hour++) {
        // Format time as HH:00
        std::stringstream timeStr;
        timeStr << std::setw(2) << std::setfill('0') << hour << ":00";
        csvFile << timeStr.str();

        // Write data for each day at this hour
        for (const auto& day : days) {
            csvFile << ",";

            // Check if there's a schedule item for this hour and day
            auto key = std::make_pair(hour, day);
            if (scheduleData.find(key) != scheduleData.end()) {
                // Escape commas in the content by wrapping in quotes
                csvFile << "\"" << scheduleData[key] << "\"";
            }
        }

        csvFile << std::endl;
    }

    csvFile.close();
    std::cout << "Schedule successfully saved to " << filePath << std::endl;
}