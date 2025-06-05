#include "validate_courses.h"
#include <algorithm>
#include <sstream>

OptimizedSlot::OptimizedSlot(const string& start, const string& end, const string& id)
        : start_time(start), end_time(end), course_id(id) {
    start_minutes = toMinutes(start);
    end_minutes = toMinutes(end);
}

bool OptimizedSlot::overlapsWith(const OptimizedSlot& other) const {
    return (start_minutes < other.end_minutes && other.start_minutes < end_minutes);
}

vector<string> validate_courses(vector<Course> courses) {
    BuildingSchedule schedule;
    vector<string> errors;

    errors.reserve(50);

    Logger::get().logInfo("Starting optimized validation of " + to_string(courses.size()) + " courses");

    size_t processed = 0;
    for (const auto& course : courses) {
        Logger::get().logInfo("Processing course: " + course.raw_id);

        processSessionGroups(course.Lectures, course.raw_id, schedule, errors);
        processSessionGroups(course.labs, course.raw_id, schedule, errors);
        processSessionGroups(course.Tirgulim, course.raw_id, schedule, errors);

        processed++;
        if (processed % 50 == 0) {
            Logger::get().logInfo("Processed " + to_string(processed) + "/" +
                                  to_string(courses.size()) + " courses. Found " +
                                  to_string(errors.size()) + " conflicts so far.");
        }
    }

    Logger::get().logInfo("Validation completed. Processed " + to_string(processed) +
                          " courses with " + to_string(errors.size()) + " conflicts found");

    return errors;
}

void processSessionGroups(const vector<Group>& groups, const string& courseId,
                          BuildingSchedule& schedule, vector<string>& errors) {
    for (const auto& group : groups) {
        for (const auto& session : group.sessions) {
            processSession(session, courseId, schedule, errors);
        }
    }
}

void processSession(const Session& session, const string& courseId,
                    BuildingSchedule& schedule, vector<string>& errors) {
    if (session.building_number.empty() || session.room_number.empty()) {
        errors.push_back("Course " + courseId + " has invalid room information");
        return;
    }

    if (session.start_time.empty() || session.end_time.empty()) {
        errors.push_back("Course " + courseId + " has invalid time information");
        return;
    }

    RoomKey roomKey = createRoomKey(session.building_number, session.room_number);

    RoomSchedule& roomSchedule = schedule[roomKey];
    DaySlots& daySlots = roomSchedule[session.day_of_week];

    OptimizedSlot newSlot(session.start_time, session.end_time, courseId);

    if (newSlot.start_minutes == -1 || newSlot.end_minutes == -1) {
        errors.push_back("Course " + courseId + " has invalid time format");
        return;
    }

    if (newSlot.start_minutes >= newSlot.end_minutes) {
        errors.push_back("Course " + courseId + " has start time after end time");
        return;
    }

    for (const auto& existingSlot : daySlots) {
        if (newSlot.overlapsWith(existingSlot)) {
            stringstream errorMsg;
            errorMsg << "Course " << courseId << " overlaps with " << existingSlot.course_id
                     << " in " << session.building_number << "-" << session.room_number
                     << " on day " << session.day_of_week
                     << " (" << session.start_time << "-" << session.end_time
                     << " vs " << existingSlot.start_time << "-" << existingSlot.end_time << ")";
            errors.push_back(errorMsg.str());
        }
    }

    daySlots.push_back(move(newSlot));
}

string createRoomKey(const string& building, const string& room) {
    return building + "_" + room;
}

int toMinutes(const string& timeStr) {
    if (timeStr.empty()) {
        return -1;
    }

    size_t colonPos = timeStr.find(':');
    if (colonPos == string::npos) {
        return -1;
    }

    try {
        string hourStr = timeStr.substr(0, colonPos);
        string minuteStr = timeStr.substr(colonPos + 1);

        int hours = stoi(hourStr);
        int minutes = stoi(minuteStr);

        if (hours < 0 || hours > 23 || minutes < 0 || minutes > 59) {
            return -1;
        }

        return hours * 60 + minutes;
    } catch (const exception&) {
        return -1;
    }
}

bool isOverLapping(const OptimizedSlot& s1, const OptimizedSlot& s2) {
    return s1.overlapsWith(s2);
}