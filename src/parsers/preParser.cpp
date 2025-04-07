#include "parsers/preParser.h"
#include <fstream>
#include <sstream>
#include <iostream>
#include <unordered_map>
#include <unordered_set>
#include <vector>
#include <string>

using namespace std;

// helper function for simple 24-hour time validation
bool isValidTime(const string& time) {
    if (time.size() != 5 || time[2] != ':') return false;

    try {
        int hour = stoi(time.substr(0, 2));
        int minute = stoi(time.substr(3, 2));

        return hour >= 0 && hour < 24 && minute >= 0 && minute < 60;
    } catch (...) {
        return false;
    }
}

// Reads user input ID pairs from file (e.g., student/course ID)
unordered_set<int> readUserCourseIDsFromFile(const string& filename) {
    ifstream inputFile(filename);
    unordered_set<int> courseIDs;

    if (!inputFile) {
        cerr << "Could not open the file: " << filename << endl;
        return courseIDs;
    }

    string line;
    while (getline(inputFile, line)) {
        istringstream iss(line);
        string token;
        while (iss >> token) {
            try {
                int id = stoi(token);
                courseIDs.insert(id);
            } catch (...) {
                cerr << "Invalid course ID: " << token << endl;
            }
        }
    }

    inputFile.close();
    return courseIDs;
}

// Parses full course DB from input stream
vector<Course> parseCourseDB(const string& path) {
    ifstream file(path);
    if (!file.is_open()) {
        cerr << "Cannot open file: " << path << endl;
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
            cerr << "Error: Missing course ID after name at line " << line_number << endl;
            break;
        }
        line_number++;
        try {
            c.id = stoi(line);
        } catch (const exception& e) {
            cerr << "Error: Invalid course ID at line " << line_number << ": " << line << " — " << e.what() << endl;
            continue;
        }

        if (courseDB.find(c.id) != courseDB.end()) {
            cerr << "Warning: Duplicate course ID " << c.id << " at line " << line_number << ". Skipping." << endl;
            continue;
        }

        // 3. Teacher name
        if (!getline(file, line)) {
            cerr << "Error: Missing teacher name at line " << line_number << " for course ID " << c.id << endl;
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
                    cerr << "Warning: Unknown session format at line " << line_number << ": " << line << endl;
                }
            } catch (const exception& e) {
                cerr << "Error parsing session line at line " << line_number << ": " << e.what() << endl;
            }
        }

        courseDB[c.id] = c;
        course_count++;
    }

    if (course_count == 0) {
        cerr << "Warning: No valid courses found in the input." << endl;
    } else {
        cout << "Successfully parsed " << course_count << " courses." << endl;
    }

    unordered_set<int> userRequestedIDs = readUserCourseIDsFromFile("../data/userInput.txt");

    if (userRequestedIDs.empty()) {
        cerr << "Error: No valid user course IDs found in userInput.txt." << endl;
        return {};
    }

    vector<Course> userSelectedCourses;
    for (const auto& [id, course] : courseDB) {
        if (userRequestedIDs.find(id) != userRequestedIDs.end()) {
            userSelectedCourses.push_back(course);
        }
    }

    if (userSelectedCourses.size() > 7) {
        cerr << "Error: User selected more than 7 valid courses (" << userSelectedCourses.size() << "). Limit is 7." << endl;
        return {};
    }

    if (userSelectedCourses.empty()) {
        cerr << "Error: No matching courses from user input exist in course database." << endl;
        return {};
    }

    cout << "User selected " << userSelectedCourses.size() << " valid courses." << endl;
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
        cerr << "Error parsing session line: \"" << line << "\" — " << e.what() << endl;
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
            cerr << "Skipping malformed session part: \"" << part << "\"" << endl;
        }
        line = "S," + line.substr(pos + 3);
    }

    try {
        sessions.push_back(parseSingleSession(line));
    } catch (...) {
        cerr << "Skipping malformed session part: \"" << line << "\"" << endl;
    }

    return sessions;
}
