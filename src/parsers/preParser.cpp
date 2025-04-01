#include "parsers/preParser.h"

using namespace std;

vector<Course> mainParse(const string& path) {
    ifstream fin(path);
    if (!fin.is_open()) {
        cerr << "Cannot open file V1.0CourseDB.txt" << endl;
        return {};
    }
    unordered_map<int, Course> courseDB;
    string line;

    while (getline(fin, line)) {
        if (line == "$$$$" || line.empty())
            continue;
        Course c;
        c.name = line;
        if (!getline(fin, line) || line.empty()) continue;
        try {
            c.id = stoi(line);
        } catch (...) {
            cerr << "Error parsing course ID: " << line << endl;
            continue;
        }
        if (!getline(fin, c.teacher)) break;

        while (getline(fin, line)) {
            if (line == "$$$$") break;
            if (line.rfind("L S", 0) == 0) {
                auto lectures = parseMultipleSessions(line.substr(2));
                c.Lectures.insert(c.Lectures.end(), lectures.begin(), lectures.end());
            } else if (line.rfind("T S", 0) == 0) {
                auto tutorials = parseMultipleSessions(line.substr(2));
                c.Tirgulim.insert(c.Tirgulim.end(), tutorials.begin(), tutorials.end());
            } else if (line.rfind("M S", 0) == 0) {
                auto labs = parseMultipleSessions(line.substr(2));
                c.labs.insert(c.labs.end(), labs.begin(), labs.end());
            }
        }

        courseDB.insert({ c.id, c });
    }
    fin.close();

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

    getline(ss, token, ','); // "S"
    getline(ss, token, ','); 
    s.day_of_week = stoi(token);

    getline(ss, token, ',');
    s.start_time = token;

    getline(ss, token, ',');
    s.end_time = token;

    getline(ss, token, ',');
    s.building_number = token;

    getline(ss, token, ',');
    s.room_number = token;

    return s;
}

// Splits "L S,1,... S,2,..." into multiple sessions
vector<Session> parseMultipleSessions(string line) {
    vector<Session> sessions;
    // Remove the first character, which is "L" or "T"
    line = line.substr(1);

    size_t pos;
    while ((pos = line.find(" S,")) != string::npos) {
        string part = line.substr(0, pos);
        sessions.push_back(parseSingleSession(part));
        // Keep "S," on the remaining part so parseSingleSession() works as intended
        line = "S," + line.substr(pos + 3);
    }
    sessions.push_back(parseSingleSession(line));
    return sessions;
}

