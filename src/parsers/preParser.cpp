#include "parsers/preParser.h"
#include "logs/logger.h"

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

// Reads user input ID's from file (e.g., student/course ID)
unordered_set<string> readSelectedCourseIDs(const string& filename) {
    ifstream inputFile(filename);
    unordered_set<string> courseIDs;
    unordered_set<string> seenIDs;

    if (!inputFile) {
        ostringstream message;
        message << "Could not open the file: " << filename;
        Logger::get().logError(message.str());

        return courseIDs;
    }

    string line;
    while (getline(inputFile, line)) {
        istringstream iss(line);
        string token;

        while (iss >> token) {
            // Validate ID format
            if (!validateID(token)) continue;

            // Warn on duplicates
            if (seenIDs.find(token) != seenIDs.end()) {
                ostringstream message;
                message << "Duplicate course ID found in user input: " << token;
                Logger::get().logWarning(message.str());

                continue;
            }

            // Add to both sets
            seenIDs.insert(token);
            courseIDs.insert(token);

            // Enforce limit of 7 total valid courses
            if (courseIDs.size() > 7) {
                Logger::get().logError("More than 7 course IDs selected. Limit is 7.");

                courseIDs.clear(); // discard everything
                inputFile.close();
                return courseIDs;
            }
        }
    }

    inputFile.close();
    return courseIDs;
}

// Parses full course DB from input stream
vector<Course> parseCourseDB(const string& path, const string& userInput) {
    fstream file(path);
    if (!file.is_open()) {
        ostringstream message;
        message << "Cannot open file: " << path;
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

        // 4. Session lines
        while (getline(file, line)) {
            line_number++;
            if (line.empty() || line == "$$$$") break;

            try {
                if (line.rfind("L S", 0) == 0) {
                    auto lectures = parseMultipleSessions(line.substr(2));
                    c.Lectures.insert(c.Lectures.end(), lectures.begin(), lectures.end());
                } else if (line.rfind("T S", 0) == 0) {
                    auto tirgul = parseMultipleSessions(line.substr(2));
                    c.Tirgulim.insert(c.Tirgulim.end(), tirgul.begin(), tirgul.end());
                } else if (line.rfind("M S", 0) == 0) {
                    auto labs = parseMultipleSessions(line.substr(2));
                    c.labs.insert(c.labs.end(), labs.begin(), labs.end());
                } else {
                    ostringstream message;
                    message << "Unknown session format at line " << line_number << ": " << line;
                    Logger::get().logWarning(message.str());
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
    } else {
        ostringstream message;
        message << "Successfully parsed " << course_count << " courses.";
        Logger::get().logInfo(message.str());
    }

    unordered_set<string> rawIDs = readSelectedCourseIDs(userInput);
    unordered_set<int> userRequestedIDs;

    for (const auto& strID : rawIDs) {
        if (!validateID(strID)) {
            ostringstream message;
            message << "Invalid course ID in validUserInput.txt (not an int): " << strID;
            Logger::get().logError(message.str());

            continue;
        }

        int id = stoi(strID);

        // Check if course exists in courseDB
        if (courseDB.find(id) == courseDB.end()) {
            ostringstream message;
            message << id << " this course does not exist";
            Logger::get().logError(message.str());

            continue;
        }

        userRequestedIDs.insert(id);
    }

    if (userRequestedIDs.empty()) {
        Logger::get().logError("No valid user course IDs found in validUserInput.txt.");

        return {};
    }

    vector<Course> userSelectedCourses;
    for (const auto& [id, course] : courseDB) {
        if (userRequestedIDs.find(id) != userRequestedIDs.end()) {
            userSelectedCourses.push_back(course);
        }
    }

    if (userSelectedCourses.size() > 7) {
        ostringstream message;
        message << "Error: User selected more than 7 valid courses (" << userSelectedCourses.size() << "). Limit is 7.";
        Logger::get().logError(message.str());

        return {};
    }

    if (userSelectedCourses.empty()) {
        Logger::get().logError("No matching courses from user input exist in course database.");

        return {};
    }

    ostringstream message;
    message << "User selected " << userSelectedCourses.size() << " valid courses.";
    Logger::get().logInfo(message.str());

    return userSelectedCourses;

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
            throw invalid_argument("Invalid day of week: " + token);
        }

        getline(ss, token, ',');
        s.start_time = token;

        getline(ss, token, ',');
        s.end_time = token;

        // ✅ Add simple time format checks
        if (!isValidTime(s.start_time) || !isValidTime(s.end_time)) {
            throw invalid_argument("Invalid time format: " + s.start_time + ", " + s.end_time);
        }

        if (s.start_time >= s.end_time) {
            throw invalid_argument("Start time must be before end time: " + s.start_time + " >= " + s.end_time);
        }

        getline(ss, token, ',');
        s.building_number = token;

        getline(ss, token, ',');
        s.room_number = token;
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
    line = line.substr(1); // remove "L", "T", or "M"

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
