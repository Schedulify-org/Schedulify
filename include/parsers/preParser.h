#ifndef PRE_PARSER_H
#define PRE_PARSER_H

#include <string>
#include <vector>
#include <sstream>
#include <fstream>
#include <iostream>
#include <unordered_map>
#include <unordered_set>

using namespace std;

class Session {
public:
    int day_of_week;
    string start_time;
    string end_time;
    string building_number;
    string room_number;
};

class Course {
public:
    int id;
    string raw_id;
    string name;
    string teacher;
    vector<Session> Lectures;
    vector<Session> Tirgulim;
    vector<Session> labs;
};

// Parses full course DB from input stream
vector<Course> parseCourseDB(const string& path);

unordered_set<string> readSelectedCourseIDs(const string& filename);

// Parses one "S,day,start,end,building,room"
Session parseSingleSession(const string& line);

// Splits "L S,1,... S,2,..." into multiple sessions
vector<Session> parseMultipleSessions(string line);

#endif // PRE_PARSER_H