#include "../include/main_app.h"

int main_app(const string& inputPath, const string& outputPath){

    vector<Course> courses = mainParse(inputPath);

    printCourses(courses);

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
