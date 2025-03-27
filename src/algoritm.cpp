#include <iostream>
#include <fstream>
#include <unordered_map>
#include <string>
#include <vector>
#include <sstream> // Assumed to provide Session and possibly parsing functions

using namespace std;

// If your parser.h does not define Session, then use this definition:
class Session {
public:
    int day_of_week;
    string start_time;
    string end_time;
    string building_number;
    string room_number;
};

// Class representing a course with its sessions.
class Course {
public:
    int id;
    string name;
    string teacher;
    vector<Session> Lectures;
    vector<Session> Tirgulim;  // Tutorials
    vector<Session> labs;      // Labs (if available)
};

// Global course database.
unordered_map<int, Course> courseDB;

// Parses one "S,day,start,end,building,room" into a Session.
Session parseSingleSession(const string& line) {
    Session s;
    stringstream ss(line);
    string token;
    getline(ss, token, ','); // Skip "S"
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

// Splits a string containing multiple sessions (e.g., "L S,1,16:00,17:00,1100,22 S,2,16:00,17:00,1100,42")
// into a vector of Session objects.
vector<Session> parseMultipleSessions(string line) {
    vector<Session> sessions;
    // Remove the first character (L, T, or M)
    line = line.substr(1);
    size_t pos;
    while ((pos = line.find(" S,")) != string::npos) {
        string part = line.substr(0, pos);
        sessions.push_back(parseSingleSession(part));
        // Prepend "S," to the remainder so that parseSingleSession works correctly.
        line = "S," + line.substr(pos + 3);
    }
    sessions.push_back(parseSingleSession(line));
    return sessions;
}

// ---------------- Schedule Building Code ----------------

// Structure representing a selection for a course – exactly one lecture must be chosen,
// and if available, one tutorial and one lab (using pointers; nullptr if not chosen).
struct CourseSelection {
    int courseId;
    const Session* lecture;
    const Session* tutorial; // nullptr if none
    const Session* lab;      // nullptr if none
};

// Structure representing a complete schedule – a collection of selections (one per course).
struct Schedule {
    vector<CourseSelection> selections;
};

// Global vector containing all possible schedules (across all courses).
vector<Schedule> allPossibleSchedules;

// Function to convert a time string (HH:MM) into the number of minutes from midnight.
int timeToMinutes(const string& t) {
    int colonPos = t.find(':');
    int hours = stoi(t.substr(0, colonPos));
    int minutes = stoi(t.substr(colonPos + 1));
    return hours * 60 + minutes;
}

// Checks if two sessions conflict (they overlap if on the same day and their times overlap).
bool isOverlap(const Session* s1, const Session* s2) {
    if (s1->day_of_week != s2->day_of_week)
        return false;
    int start1 = timeToMinutes(s1->start_time);
    int end1   = timeToMinutes(s1->end_time);
    int start2 = timeToMinutes(s2->start_time);
    int end2   = timeToMinutes(s2->end_time);
    return (start1 < end2 && start2 < end1);
}

// Returns all valid combinations of sessions for a specific course.
// If the course provides tutorials then a valid combination must include one;
// if it provides labs then a valid combination must include one.
// (Only if a course does not provide tutorials or labs is it acceptable to have a combination with only a lecture.)
vector<CourseSelection> getValidCourseCombinations(int courseId,
    const vector<const Session*>& lectures, 
    const vector<const Session*>& tutorials, 
    const vector<const Session*>& labs) {
    
    vector<CourseSelection> validCombinations;
    for (const auto* lecture : lectures) {
        // If tutorials exist, we must choose one.
        if (!tutorials.empty()) {
            for (const auto* tutorial : tutorials) {
                if (isOverlap(lecture, tutorial))
                    continue;  // Skip if lecture and tutorial overlap.
                // If labs exist, we must choose one.
                if (!labs.empty()) {
                    for (const auto* lab : labs) {
                        if (isOverlap(lecture, lab) || isOverlap(tutorial, lab))
                            continue; // Skip if there is any overlap.
                        validCombinations.push_back({courseId, lecture, tutorial, lab});
                    }
                } else {
                    // No labs exist, so combination must include lecture and tutorial.
                    validCombinations.push_back({courseId, lecture, tutorial, nullptr});
                }
            }
        }
        // If tutorials do NOT exist but labs do, then combination must include a lab.
        else if (!labs.empty()) {
            for (const auto* lab : labs) {
                if (isOverlap(lecture, lab))
                    continue;
                validCombinations.push_back({courseId, lecture, nullptr, lab});
            }
        }
        // If neither tutorials nor labs exist, then only a lecture is available.
        else {
            validCombinations.push_back({courseId, lecture, nullptr, nullptr});
        }
    }
    return validCombinations;
}

// Returns all sessions in a CourseSelection (for conflict checking).
vector<const Session*> getSessions(const CourseSelection& cs) {
    vector<const Session*> sessions;
    sessions.push_back(cs.lecture);
    if (cs.tutorial != nullptr)
        sessions.push_back(cs.tutorial);
    if (cs.lab != nullptr)
        sessions.push_back(cs.lab);
    return sessions;
}

// Checks for conflict between the sessions in current selections and a new selection.
bool noConflict(const vector<CourseSelection>& current, const CourseSelection& cs) {
    vector<const Session*> newSessions = getSessions(cs);
    for (const auto& sel : current) {
        vector<const Session*> existingSessions = getSessions(sel);
        for (const auto* s1 : existingSessions) {
            for (const auto* s2 : newSessions) {
                if (isOverlap(s1, s2))
                    return false;
            }
        }
    }
    return true;
}

// Backtracking function: For each course (represented in courseOptions),
// choose one valid CourseSelection if it doesn't conflict with the already chosen selections.
void backtrack(int index, vector<CourseSelection>& current, const vector<vector<CourseSelection>>& courseOptions) {
    if (index == courseOptions.size()) {
        Schedule sch;
        sch.selections = current;
        allPossibleSchedules.push_back(sch);
        return;
    }
    for (const auto& cs : courseOptions[index]) {
        if (noConflict(current, cs)) {
            current.push_back(cs);
            backtrack(index + 1, current, courseOptions);
            current.pop_back();
        }
    }
}

int main() {
    // ----- Step 1: Read courses from file "V1.0CourseDB.txt" -----
    ifstream fin("V1.0CourseDB.txt");
    if (!fin.is_open()) {
        cerr << "Cannot open file V1.0CourseDB.txt" << endl;
        return 1;
    }
    string line;
    while (getline(fin, line)) {
        if (line == "$$$$" || line.empty())
            continue;
        Course c;
        c.name = line;
        if (!getline(fin, line)) break;
        c.id = stoi(line);
        if (!getline(fin, c.teacher)) break;
        while (getline(fin, line)) {
            if (line == "$$$$")
                break;
            if (line.rfind("L S", 0) == 0) {
                vector<Session> lectures = parseMultipleSessions(line.substr(2));
                c.Lectures.insert(c.Lectures.end(), lectures.begin(), lectures.end());
            } else if (line.rfind("T S", 0) == 0) {
                vector<Session> tutorials = parseMultipleSessions(line.substr(2));
                c.Tirgulim.insert(c.Tirgulim.end(), tutorials.begin(), tutorials.end());
            } else if (line.rfind("M S", 0) == 0) {
                vector<Session> labs = parseMultipleSessions(line.substr(2));
                c.labs.insert(c.labs.end(), labs.begin(), labs.end());
            }
        }
        courseDB.insert({ c.id, c });
    }
    fin.close();

    // Create a vector of courses from courseDB.
    vector<Course> courses;
    for (const auto& kv : courseDB)
        courses.push_back(kv.second);

    // ----- Step 2: For each course, build valid session combinations -----
    vector<vector<CourseSelection>> courseOptions;
    for (const auto& course : courses) {
        vector<const Session*> lecturePtrs, tutorialPtrs, labPtrs;
        for (const auto& s : course.Lectures)
            lecturePtrs.push_back(&s);
        for (const auto& s : course.Tirgulim)
            tutorialPtrs.push_back(&s);
        for (const auto& s : course.labs)
            labPtrs.push_back(&s);
        vector<CourseSelection> validCombinations = getValidCourseCombinations(course.id, lecturePtrs, tutorialPtrs, labPtrs);
        courseOptions.push_back(validCombinations);
    }

    // ----- Step 3: Use backtracking to combine selections across all courses -----
    vector<CourseSelection> currentSchedule;
    backtrack(0, currentSchedule, courseOptions);

    // Print the total valid schedules found.
    cout << "Total valid schedules: " << allPossibleSchedules.size() << endl;

    // Optionally, print each schedule.
    for (size_t i = 0; i < allPossibleSchedules.size(); i++) {
        cout << "\nSchedule " << i + 1 << ":\n";
        for (const auto& cs : allPossibleSchedules[i].selections) {
            cout << "Course " << cs.courseId << ":\n";
            cout << "  Lecture: Day " << cs.lecture->day_of_week
                 << " " << cs.lecture->start_time << "-" << cs.lecture->end_time
                 << " in building " << cs.lecture->building_number
                 << " room " << cs.lecture->room_number << endl;
            if (cs.tutorial != nullptr) {
                cout << "  Tutorial: Day " << cs.tutorial->day_of_week
                     << " " << cs.tutorial->start_time << "-" << cs.tutorial->end_time
                     << " in building " << cs.tutorial->building_number
                     << " room " << cs.tutorial->room_number << endl;
            }
            if (cs.lab != nullptr) {
                cout << "  Lab: Day " << cs.lab->day_of_week
                     << " " << cs.lab->start_time << "-" << cs.lab->end_time
                     << " in building " << cs.lab->building_number
                     << " room " << cs.lab->room_number << endl;
            }
        }
        cout << "---------------------------" << endl;
    }

    return 0;
}
