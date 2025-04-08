#include "main/main_app.h"

int main_app(const string& inputPath, const string& modifiedOutputPath){

    vector<Course> courses = parseCourseDB(inputPath);

    printCourses(courses);

    ScheduleBuilder builder;
    auto schedules = builder.build(courses);

    printSchedules(courses);

    exportSchedulesByDayText(schedules, modifiedOutputPath);

    return 0;
}

void printSessionList(const vector<Session>& sessions) {
    for (const auto& s : sessions) {
        cout << "  Day: " << s.day_of_week
             << ", Start: " << s.start_time
             << ", End: " << s.end_time
             << ", Building: " << s.building_number
             << ", Room: " << s.room_number << endl;
    }
}

void printCourses(const vector<Course>& courses) {
    for (const auto& c : courses) {
        cout << "Course: " << c.name << "\n";
        cout << "ID: " << c.id << "\n";
        cout << "Teacher: " << c.teacher << "\n";

        cout << "Lectures:\n";
        printSessionList(c.Lectures);

        cout << "Tirgulim:\n";
        printSessionList(c.Tirgulim);

        cout << "Labs:\n";
        printSessionList(c.labs);

        cout << "---------------------------\n";
    }
}

void printSchedules(const vector<Course>& courses) {
    ScheduleBuilder builder;
    auto schedules = builder.build(courses);

    cout << "Total valid schedules: " << schedules.size() << endl;

    //example for extraction data from the schedules variable -- IMPORTANT
    for (size_t i = 0; i < schedules.size(); i++) {
        cout << "\nSchedule " << i + 1 << ":" << endl;
        for (const auto& cs : schedules[i].selections) {
            cout << "Course " << cs.courseId << ":" << endl;
            cout << "Course ID " << cs.courseId << ":" << endl;
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
}

