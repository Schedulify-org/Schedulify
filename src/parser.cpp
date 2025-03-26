#include <iostream>
#include <fstream>
#include <unordered_map>
#include <string>
#include <vector>
#include <sstream>

using namespace std;

class Session {
public:
    int day_of_week;
    string start_time;
    string end_time;
    string building_number;
    string room_number;
};

class course {
public:
    int id;
    string name;
    string Teacher;
    vector<Session> Lectures;
    vector<Session> Tirgulim;
};

unordered_map<int, course> courseDB;

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

int main() {
    ifstream Courses("V1.0CourseDB.txt");
    if (!Courses.is_open()) {
        cerr << "Failed to open file." << endl;
        return 1;
    }

    string line;

    // Read until we run out of lines
    while (true) {
        // 1) Try reading a line; if no line, break.
        if (!getline(Courses, line)) break;

        // 2) If line is $$$$ or empty, skip it and continue.
        if (line == "$$$$" || line.empty()) {
            continue;
        }

        // Now 'line' should hold the course name
        course a;
        a.name = line;

        // Next line should hold the numeric course ID
        if (!getline(Courses, line)) break;
        a.id = stoi(line);

        // Next line should hold the teacher's name
        if (!getline(Courses, a.Teacher)) break;

        // Read session lines until we see "$$$$" or EOF
        while (true) {
            // Attempt to read next line
            if (!getline(Courses, line)) {
                // If we can't read anymore, break out of the session loop
                break;
            }
            if (line == "$$$$") {
                // Session data for this course ends here
                break;
            }

            // Check if it's an L S or T S
            if (line.rfind("L S", 0) == 0) {
                auto lectures = parseMultipleSessions(line.substr(2));
                a.Lectures.insert(a.Lectures.end(), lectures.begin(), lectures.end());
            } 
            else if (line.rfind("T S", 0) == 0) {
                auto tirgul = parseMultipleSessions(line.substr(2));
                a.Tirgulim.insert(a.Tirgulim.end(), tirgul.begin(), tirgul.end());
            }
        }

        // Insert the course into our DB
        courseDB.insert({ a.id, a });
    }

    // Debug print
    for (const auto& [id, c] : courseDB) {
        cout << "Course ID: " << id << endl;
        cout << "Name: " << c.name << endl;
        cout << "Teacher: " << c.Teacher << endl;

        cout << "Lectures:" << endl;
        for (const auto& lec : c.Lectures) {
            cout << "  Day: S" << lec.day_of_week
                 << ", Start: " << lec.start_time 
                 << ", End: " << lec.end_time 
                 << ", Building: " << lec.building_number 
                 << ", Room: " << lec.room_number << endl;
        }

        cout << "Tirguim:" << endl;
        for (const auto& tirg : c.Tirgulim) {
            cout << "  Day: S" << tirg.day_of_week 
                 << ", Start: " << tirg.start_time 
                 << ", End: " << tirg.end_time 
                 << ", Building: " << tirg.building_number 
                 << ", Room: " << tirg.room_number << endl;
        }

        cout << "-----------------------" << endl;
    }

    return 0;
}
