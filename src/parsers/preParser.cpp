#include "parsers/preParser.h"

using namespace std;

// Parses full course DB from input stream
vector<Course> parseCourseDB(const string& path) {

    ifstream file(path);
    if (!file.is_open()) {
        cerr << "Cannot open file V1.0CourseDB.txt" << endl;
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

        // Check for duplicate ID
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

        // 5. Store course
        courseDB[c.id] = c;
        course_count++;
    }

    if (course_count == 0) {
        cerr << "Warning: No valid courses found in the input." << endl;
    } else {
        cout << "Successfully parsed " << course_count << " courses." << endl;
    }

    vector<Course> courses;
    courses.reserve(courseDB.size());
    for (const auto& kv : courseDB)
        courses.push_back(kv.second);

    return courses;
}

// Parses one "S,day,start,end,building,room"
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

        getline(ss, token, ',');
        s.building_number = token;

        getline(ss, token, ',');
        s.room_number = token;
    } catch (const exception& e) {
        cerr << "Error parsing session line: \"" << line << "\" — " << e.what() << endl;
        throw;  // rethrow to let caller decide
    }

    return s;
}

// Splits "L S,1,... S,2,..." into multiple sessions
vector<Session> parseMultipleSessions(string line) {
    vector<Session> sessions;
    line = line.substr(1); // Remove the first character, which is "L" or "T"


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