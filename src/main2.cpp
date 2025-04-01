
#include <iostream>
#include <fstream>
#include "../include/parser.h"
#include "../include/ScheduleBuilder.h"
#include "../include/schedule_types.h"
#include "../include/TimeUtils.h"
#include "../include/CourseLegalComb.h"

using namespace std;

int main() {
    ifstream fin("V1.0CourseDB.txt");
    if (!fin.is_open()) {
        cerr << "Cannot open file V1.0CourseDB.txt" << endl;
        return 1;
    }

    string line;
    unordered_map<int, Course> courseDB;

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
    for (const auto& kv : courseDB)
        courses.push_back(kv.second);

    ScheduleBuilder builder;
    auto schedules = builder.build(courses);

    cout << "Total valid schedules: " << schedules.size() << endl;

    //example for extraction data from the schedules variable -- IMPORTANT
    for (size_t i = 0; i < schedules.size(); i++) {
        cout << "\nSchedule " << i + 1 << ":" << endl;
        for (const auto& cs : schedules[i].selections) {
            cout << "Course " << cs.courseId << ":" << endl;
            if (cs.lecture) {
                cout << "  Lecture: Day " << cs.lecture->day_of_week
                     << " " << cs.lecture->start_time << "-" << cs.lecture->end_time
                     << " in building " << cs.lecture->building_number
                     << " room " << cs.lecture->room_number << endl;
            }
            if (cs.tutorial) {
                cout << "  Tutorial: Day " << cs.tutorial->day_of_week
                     << " " << cs.tutorial->start_time << "-" << cs.tutorial->end_time
                     << " in building " << cs.tutorial->building_number
                     << " room " << cs.tutorial->room_number << endl;
            }
            if (cs.lab) {
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
