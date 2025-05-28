#ifndef PRE_PARSER_H
#define PRE_PARSER_H

#include "model_interfaces.h"

#include <string>
#include <vector>
#include <sstream>
#include <fstream>
#include <iostream>
#include <filesystem>
#include <unordered_map>
#include <unordered_set>
#include <algorithm>

using namespace std;



// Function declarations
bool isValidTime(const string& time);

// Parses full course DB from input stream - returns all courses without filtering
vector<Course> parseCourseDB(const string& path);

// Parses one "S,day,start,end,building,room"
Session parseSingleSession(const string& line);

// Splits "L S,1,... S,2,..." into multiple sessions
vector<Session> parseMultipleSessions(string line);

bool isInteger(const std::string& s);

bool validateID(const string& raw_id);

bool validateLocation(const string &location, int type);

vector<Course> extractUserChoice(const string& path, const vector<Course>& courses);

unordered_set<string> readSelectedCourseIDs(const string& filename);

#endif // PRE_PARSER_H