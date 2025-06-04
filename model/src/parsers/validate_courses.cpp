#include "validate_courses.h"

vector<string> validate_courses(vector<Course> courses) {
    vector<Building> Buildings = {};
	vector<string> errors;
    for (auto course : courses) {
        // Process Lectures
        for (auto group : course.Lectures) {
            for (auto session : group.sessions) {
                // This will now work with your operator==
                auto buildingIt = std::find(Buildings.begin(), Buildings.end(), session.building_number);

                if (buildingIt == Buildings.end()) {
                    // Building doesn't exist, create new one
                    Slot s = {session.start_time, session.end_time, course.raw_id};
                    Day_of_week day = {session.day_of_week, {s}}; // Create day with the slot
                    Room r = {session.room_number, {day}}; // Create room with the day
                    Building b = {session.building_number, {r}}; // Create building with the room
                    Buildings.push_back(b);
                } else {
                    // Building exists, check if room exists
                    auto roomIt = std::find(buildingIt->Room.begin(), buildingIt->Room.end(), session.room_number);

                    if (roomIt == buildingIt->Room.end()) {
                        // Room doesn't exist in building, create new room
                        Slot s = {session.start_time, session.end_time, course.raw_id};
                        Day_of_week day = {session.day_of_week, {s}};
                        Room r = {session.room_number, {day}};
                        buildingIt->Room.push_back(r);
                    } else {
                        // Room exists, check if day exists
                        auto dayIt = std::find_if(roomIt->Day_of_week.begin(), roomIt->Day_of_week.end(),
                            [&session](const Day_of_week& day) {
                                return day.id == session.day_of_week;
                            });

                        if (dayIt == roomIt->Day_of_week.end()) {
                            // Day doesn't exist, create new day
                            Slot s = {session.start_time, session.end_time, course.raw_id};
                            Day_of_week day = {session.day_of_week, {s}};
                            roomIt->Day_of_week.push_back(day);
                        } else {
                            // Day exists, just add the slot
                            Slot s = {session.start_time, session.end_time, course.raw_id};
                            for (Slot st : dayIt->Slot) {
                              if(isOverLapping(s, st)) errors.push_back("Course " + course.raw_id + " has overlapping sessions with " + st.id);
                            }
                            dayIt->Slot.push_back(s);
                        }
                    }
                }
            }
        }

        // Process Labs
        for (auto group : course.labs) {
            for (auto session : group.sessions) {
                auto buildingIt = std::find(Buildings.begin(), Buildings.end(), session.building_number);

                if (buildingIt == Buildings.end()) {
                    // Building doesn't exist, create new one
                    Slot s = {session.start_time, session.end_time, course.raw_id};
                    Day_of_week day = {session.day_of_week, {s}};
                    Room r = {session.room_number, {day}};
                    Building b = {session.building_number, {r}};
                    Buildings.push_back(b);
                } else {
                    // Building exists, check if room exists
                    auto roomIt = std::find(buildingIt->Room.begin(), buildingIt->Room.end(), session.room_number);

                    if (roomIt == buildingIt->Room.end()) {
                        // Room doesn't exist in building, create new room
                        Slot s = {session.start_time, session.end_time, course.raw_id};
                        Day_of_week day = {session.day_of_week, {s}};
                        Room r = {session.room_number, {day}};
                        buildingIt->Room.push_back(r);
                    } else {
                        // Room exists, check if day exists
                        auto dayIt = std::find_if(roomIt->Day_of_week.begin(), roomIt->Day_of_week.end(),
                            [&session](const Day_of_week& day) {
                                return day.id == session.day_of_week;
                            });

                        if (dayIt == roomIt->Day_of_week.end()) {
                            // Day doesn't exist, create new day
                            Slot s = {session.start_time, session.end_time, course.raw_id};
                            Day_of_week day = {session.day_of_week, {s}};
                            roomIt->Day_of_week.push_back(day);
                        } else {
                            // Day exists, just add the slot

                            Slot s = {session.start_time, session.end_time, course.raw_id};
                            for (Slot st : dayIt->Slot) {
                              if(isOverLapping(s, st)) errors.push_back("Course " + course.raw_id + " has overlapping sessions with " + st.id);
                            }
                            dayIt->Slot.push_back(s);
                        }
                    }
                }
            }
        }

        // Process Tirgulim
        for (auto group : course.Tirgulim) {
            for (auto session : group.sessions) {
                auto buildingIt = std::find(Buildings.begin(), Buildings.end(), session.building_number);

                if (buildingIt == Buildings.end()) {
                    // Building doesn't exist, create new one
                    Slot s = {session.start_time, session.end_time, course.raw_id};
                    Day_of_week day = {session.day_of_week, {s}};
                    Room r = {session.room_number, {day}};
                    Building b = {session.building_number, {r}};
                    Buildings.push_back(b);
                } else {
                    // Building exists, check if room exists
                    auto roomIt = std::find(buildingIt->Room.begin(), buildingIt->Room.end(), session.room_number);

                    if (roomIt == buildingIt->Room.end()) {
                        // Room doesn't exist in building, create new room
                        Slot s = {session.start_time, session.end_time, course.raw_id};
                        Day_of_week day = {session.day_of_week, {s}};
                        Room r = {session.room_number, {day}};
                        buildingIt->Room.push_back(r);
                    } else {
                        // Room exists, check if day exists
                        auto dayIt = std::find_if(roomIt->Day_of_week.begin(), roomIt->Day_of_week.end(),
                            [&session](const Day_of_week& day) {
                                return day.id == session.day_of_week;
                            });

                        if (dayIt == roomIt->Day_of_week.end()) {
                            // Day doesn't exist, create new day
                            Slot s = {session.start_time, session.end_time, course.raw_id};
                            Day_of_week day = {session.day_of_week, {s}};
                            roomIt->Day_of_week.push_back(day);
                        } else {
                            // Day exists, just add the slot
                            Slot s = {session.start_time, session.end_time, course.raw_id};
                            for (Slot st : dayIt->Slot) {
                              if(isOverLapping(s, st)) errors.push_back("Course " + course.raw_id + " has overlapping sessions with " + st.id);
                            }

                            dayIt->Slot.push_back(s);
                        }
                    }
                }
            }
        }
    }
    return errors;
}

int toMinutes(const std::string& t) {

    size_t colonPos = t.find(':'); // Find the position of the colon separator


    // Extract hour and minute substrings
    std::string hourStr = t.substr(0, colonPos);
    std::string minuteStr = t.substr(colonPos + 1);


    // Convert to integers
    int hours = std::stoi(hourStr);
    int minutes = std::stoi(minuteStr);


    // Convert to total minutes
    return hours * 60 + minutes;
}

bool isOverLapping(const Slot &s1, const Slot &s2) {
    int start1 = toMinutes(s1.start_time);
    int end1 = toMinutes(s1.end_time);
    int start2 = toMinutes(s2.start_time);
    int end2 = toMinutes(s2.end_time);

    // Return true if the time intervals overlap
    return (start1 < end2 && start2 < end1);
}


