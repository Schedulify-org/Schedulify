#include "schedule_algorithm/TimeUtils.h"
#include "schedule_algorithm/schedule_types.h"
#include "logs/logger.h"

#include <string>

using namespace std;

// Converts a time string in the format "HH:MM" to total minutes since midnight
int TimeUtils::toMinutes(const std::string& t) {
    try {
        size_t colonPos = t.find(':'); // Find the position of the colon separator
        if (colonPos == std::string::npos) {
            // Throw error if colon is missing
            throw std::invalid_argument("Missing colon in time string: " + t);
        }

        // Extract hour and minute substrings
        std::string hourStr = t.substr(0, colonPos);
        std::string minuteStr = t.substr(colonPos + 1);

        // Check for empty parts (e.g., ":30" or "13:")
        if (hourStr.empty() || minuteStr.empty()) {
            throw std::invalid_argument("Empty hour or minute in time string: " + t);
        }

        // Convert to integers
        int hours = std::stoi(hourStr);
        int minutes = std::stoi(minuteStr);

        // Validate range for hours and minutes
        if (hours < 0 || hours > 23 || minutes < 0 || minutes > 59) {
            throw std::invalid_argument("Hour or minute out of range: " + t);
        }

        // Convert to total minutes
        return hours * 60 + minutes;
    } catch (const std::exception& e) {
        // Log any parsing or conversion errors
        Logger::get().logError("toMinutes() error: " + string(e.what()));
        throw; // Rethrow to propagate the exception
    }
}

// Determines whether two sessions overlap in time on the same day
bool TimeUtils::isOverlap(const Session* s1, const Session* s2) {
    // Check for null session pointers
    if (!s1 || !s2) {
        Logger::get().logWarning("isOverlap() received a null Session pointer.");
        return false;
    }

    try {
        // Sessions on different days cannot overlap
        if (s1->day_of_week != s2->day_of_week) return false;

        // Convert start and end times to minutes
        int start1 = toMinutes(s1->start_time);
        int end1 = toMinutes(s1->end_time);
        int start2 = toMinutes(s2->start_time);
        int end2 = toMinutes(s2->end_time);

        // Return true if the time intervals overlap
        return (start1 < end2 && start2 < end1);
    } catch (const std::exception& e) {
        // Log any exception that occurs during overlap calculation
        Logger::get().logError("isOverlap() error comparing sessions: " + string(e.what()));
        return false;
    }
}
