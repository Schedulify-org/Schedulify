#ifndef EXCEL_PARSER_H
#define EXCEL_PARSER_H

#include <iostream>
#include <vector>
#include <string>
#include <map>
#include <regex>
#include <unordered_map>
#include <sstream>
#include <algorithm>
#include <OpenXLSX.hpp>

using namespace std;
using namespace OpenXLSX;

class Session {
public:
    int day_of_week;
    string start_time;
    string end_time;
    string building_number;
    string room_number;
    string teacher;
};

class Group {
public:
    string type;
    vector<Session> sessions;
};

class Course {
public:
    int id;
    string name;
    vector<Group> lectures;
    vector<Group> tirgulim;
    vector<Group> labs;
};

class ExcelCourseParser {
private:
    unordered_map<string, int> dayMap;
    unordered_map<string, string> sessionTypeMap;

public:
    ExcelCourseParser();

    // Parse multiple rooms from single cell
    vector<string> parseMultipleRooms(const string& roomStr);

    // Parse multiple time slots and match each with corresponding room
    vector<Session> parseMultipleSessions(const string& timeSlotStr, const string& roomStr, const string& teacher);

    // Parse single session
    Session parseSingleSession(const string& timeSlotStr, const string& roomStr, const string& teacher);

    // Get session type mapping
    string getSessionType(const string& hebrewType);

    // Parse course code from full code
    pair<string, string> parseCourseCode(const string& fullCode);

    // Main parsing function
    vector<Course> parseExcelFile(const string& filename);
};

// Utility functions
string getDayName(int dayOfWeek);

#endif // EXCEL_PARSER_H