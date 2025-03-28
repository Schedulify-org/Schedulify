#include <iostream>
#include <fstream>
#include "../include/parser.h"
#include "../include/algoritm.h"
#include "../include/main.h"
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