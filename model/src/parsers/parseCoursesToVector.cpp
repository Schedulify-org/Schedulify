#include "parseCoursesToVector.h"

using namespace std;

// helper function for simple 24-hour time validation
bool isValidTime(const string& time)  {
    if (time.size() != 5 || time[2] != ':') return false;

    try {
        int hour = stoi(time.substr(0, 2));
        int minute = stoi(time.substr(3, 2));

        return hour >= 0 && hour < 24 && minute >= 0 && minute < 60;
    } catch (...) {
        return false;
    }
}

// Parses full course DB from input stream
vector<Course> parseCourseDB(const string& path) {
    filesystem::path filePath(path);
    fstream file(filePath);
    if (!file.is_open()) {
        ostringstream message;
        message << "Cannot open file: " << filePath;
        Logger::get().logError(message.str());

        return {};
    }

    unordered_map<int, Course> courseDB;
    string line;
    int line_number = 0;
    int course_count = 0;

    while (getline(file, line)) {
        line_number++;
        if (line.empty() || line == "$$$$") continue;

        Course c;

        // 1. Course name
        c.name = line;

        // 2. Course ID
        if (!getline(file, line)) {
            ostringstream message;
            message << "Error: Missing course ID after name at line " << line_number;
            Logger::get().logError(message.str());

            // Skip rest of malformed course
            while (getline(file, line)) {
                line_number++;
                if (line == "$$$$") break;
            }

            continue;
        }

        line_number++;
        if (!validateID(line)) {
            ostringstream message;
            message << "Error: Invalid course ID at line " << line_number << ": " << line;
            Logger::get().logError(message.str());

            //  Skip everything until the next course marker ($$$$)
            while (getline(file, line)) {
                line_number++;
                if (line == "$$$$") break;
            }

            continue;  //  Skip this entire course
        }

        else {
            c.id = stoi(line);
            c.raw_id = line;  // <--- store raw string version
        }

        if (courseDB.find(c.id) != courseDB.end()) {
            ostringstream message;
            message << "Duplicate course ID " << c.id << " at line " << line_number << ". Skipping.";
            Logger::get().logWarning(message.str());

            continue;
        }

        // 3. Teacher name
        if (!getline(file, line)) {
            ostringstream message;
            message << "Error: Missing teacher name at line " << line_number << " for course ID " << c.id;
            Logger::get().logError(message.str());

            break;
        }
        line_number++;
        c.teacher = line;

        // 4. Session lines - NEW STRUCTURE
        while (getline(file, line)) {
            line_number++;
            if (line.empty() || line == "$$$$") break;

            try {
                if (line.rfind("L S", 0) == 0) {
                    // Create a new Group for lectures
                    Group lectureGroup;
                    lectureGroup.type = SessionType::LECTURE;

                    // Parse all sessions in this line and add them to the group
                    auto sessions = parseMultipleSessions(line.substr(2));
                    lectureGroup.sessions = sessions;

                    // Only add the group if it has valid sessions
                    if (!lectureGroup.sessions.empty()) {
                        c.Lectures.push_back(lectureGroup);
                    }

                } else if (line.rfind("T S", 0) == 0) {
                    // Create a new Group for tirgulim
                    Group tirgulGroup;
                    tirgulGroup.type = SessionType::TUTORIAL;

                    // Parse all sessions in this line and add them to the group
                    auto sessions = parseMultipleSessions(line.substr(2));
                    tirgulGroup.sessions = sessions;

                    // Only add the group if it has valid sessions
                    if (!tirgulGroup.sessions.empty()) {
                        c.Tirgulim.push_back(tirgulGroup);
                    }

                } else if (line.rfind("M S", 0) == 0) {
                    // Create a new Group for labs
                    Group labGroup;
                    labGroup.type = SessionType::LAB;

                    // Parse all sessions in this line and add them to the group
                    auto sessions = parseMultipleSessions(line.substr(2));
                    labGroup.sessions = sessions;

                    // Only add the group if it has valid sessions
                    if (!labGroup.sessions.empty()) {
                        c.labs.push_back(labGroup);
                    }

                } else {
                    ostringstream message;
                    message << "Unknown session format at line " << line_number << ": " << line;
                    Logger::get().logWarning(message.str());
                    while (getline(file, line)) {
                        line_number++;
                        if (line == "$$$$") break;
                    }
                    break;
                }
            } catch (const exception& e) {
                ostringstream message;
                message << "Error parsing session line at line " << line_number << ": " << e.what();
                Logger::get().logError(message.str());
            }
        }

        if (validateID(c.raw_id)) {
            courseDB[c.id] = c;
            course_count++;
        }
    }

    if (course_count == 0) {
        Logger::get().logError("No valid courses found in the input.");
        return {};
    } else {
        // Check for courses with no sessions and remove them
        auto it = courseDB.begin();
        while (it != courseDB.end()) {
            const Course& course = it->second;
            if (course.Lectures.empty() && course.Tirgulim.empty() && course.labs.empty()) {
                ostringstream message;
                message << "Course: " << it->first << " has no sessions and therefore has been deleted";
                Logger::get().logError(message.str());
                it = courseDB.erase(it);
                course_count--;
            } else {
                ++it;
            }
        }

        if (course_count == 0) {
            Logger::get().logError("No valid courses found in the input.");
            return {};
        }

        ostringstream message;
        message << "Successfully parsed " << course_count << " courses.";
        Logger::get().logInfo(message.str());
    }

    // Convert courseDB map to vector (without filtering by user selection)
    vector<Course> allCourses;
    for (const auto& [id, course] : courseDB) {
        allCourses.push_back(course);
    }

    return allCourses;
}

// Parses one session string "S,day,start,end,building,room"
Session parseSingleSession(const string& line) {
    Session s;
    stringstream ss(line);
    string token;

    try {
        getline(ss, token, ','); // "S"

        getline(ss, token, ',');
        s.day_of_week = stoi(token);
        if (s.day_of_week < 1 || s.day_of_week > 7) {
            ostringstream message;
            message << "Invalid day of week: " + token;
            Logger::get().logError(message.str());
            throw invalid_argument(message.str());
        }

        getline(ss, token, ',');
        s.start_time = token;

        getline(ss, token, ',');
        s.end_time = token;

        //  Add simple time format checks
        if (!isValidTime(s.start_time) || !isValidTime(s.end_time)) {
            ostringstream message;
            message << "Invalid time format: " + s.start_time + ", " + s.end_time;
            Logger::get().logError(message.str());
            throw invalid_argument(message.str());
        }

        if (s.start_time >= s.end_time) {
            ostringstream message;
            message << "Start time must be before end time: " + s.start_time + " >= " + s.end_time;
            Logger::get().logError(message.str());
            throw invalid_argument(message.str());
        }

        getline(ss, token, ',');
        if (validateLocation(token,4)) {
            s.building_number = token;
        }
        else {
            ostringstream message;
            message << "Found invalid Session,Building number must be from 1 - 9999";
            Logger::get().logError(message.str());
            throw invalid_argument(message.str());
        }

        getline(ss, token, ',');
        if (validateLocation(token,3)) {
            s.room_number = token;
        }
        else {
            ostringstream message;
            message << "Found invalid Session,Room number must be from 1 - 999";
            Logger::get().logError(message.str());
            throw invalid_argument(message.str());

        }
    } catch (const exception& e) {
        ostringstream message;
        message << "Error parsing session line: \"" << line << "\" — " << e.what();
        Logger::get().logError(message.str());
        throw;
    }
    return s;
}

// Parses multiple sessions from string like "S,1,... S,2,..."
vector<Session> parseMultipleSessions(string line) {
    vector<Session> sessions;

    size_t pos;
    while ((pos = line.find(" S,")) != string::npos) {
        string part = line.substr(0, pos);
        try {
            sessions.push_back(parseSingleSession(part));
        } catch (...) {
            ostringstream message;
            message << "Skipping malformed session part: \"" << part << "\"";
            Logger::get().logWarning(message.str());
        }
        line = "S," + line.substr(pos + 3);
    }

    try {
        sessions.push_back(parseSingleSession(line));
    } catch (...) {
        ostringstream message;
        message << "Skipping malformed session part: \"" << line << "\"";
        Logger::get().logWarning(message.str());

    }
    return sessions;
}

bool validateID(const string& raw_id) {
    if (raw_id.size()!=5) return false;
    if (!isInteger(raw_id)) return false;
    return true;
}

bool isInteger(const std::string& s) {
    try {
        std::size_t pos;
        std::stoi(s, &pos);
        return pos == s.size(); // ensure entire string was consumed
    } catch (...) {
        return false;
    }
}

bool validateLocation(const string &location, int type) {
    if (location.size()>type || location.size()<1) return false;
    if (!isInteger(location)) return false;
    return true;
}

vector<Course> extractUserChoice(const string& path, const vector<Course>& courses) {
    fstream file(path);
    if (!file.is_open()) {
        ostringstream message;
        message << "Cannot open file: " << path;
        Logger::get().logError(message.str());

        return {};
    }

    unordered_set<string> userChoice = readSelectedCourseIDs(path);
    vector<Course> filteredCourses;

    if (userChoice.empty()) {
        Logger::get().logError("No valid selection were found in user choice extraction");

        file.close();
        return {};
    }

    // Iterate through the userChoice set and find the corresponding courses in the courses vector
    for (const auto& courseId : userChoice) {
        // Find the matching course in the courses vector using raw_id or id
        auto it = find_if(courses.begin(), courses.end(), [&](const Course& course) {
            return course.raw_id == courseId || to_string(course.id) == courseId;
        });

        // If a matching course is found, add it to the filteredCourses vector
        if (it != courses.end()) {
            filteredCourses.push_back(*it);
        }
    }

    return filteredCourses;
}

// Reads user input ID's from file (e.g., student/course ID)
unordered_set<string> readSelectedCourseIDs(const string& filename) {
    fstream file(filename);
    if (!file.is_open()) {
        ostringstream message;
        message << "Cannot open file: " << filename;
        Logger::get().logError(message.str());

        return {};
    }
    unordered_set<string> courseIDs;

    string line;
    while (getline(file, line)) {
        istringstream iss(line);
        string token;

        while (iss >> token) {
            // Validate ID format
            if (!validateID(token)) {
                ostringstream message;
                message << "Invalid course ID: " << token;
                Logger::get().logWarning(message.str());

                continue;
            }

            // Warn on duplicates
            if (courseIDs.find(token) != courseIDs.end()) {
                ostringstream message;
                message << "Duplicate course ID found in user input: " << token;
                Logger::get().logWarning(message.str());

                continue;
            }

            courseIDs.insert(token);

            if (courseIDs.size() > 7) {
                Logger::get().logError("More than 7 course IDs selected. Limit is 7.");

                courseIDs.clear(); // discard everything
                file.close();
                return courseIDs;
            }
        }
    }

    file.close();
    return courseIDs;
}