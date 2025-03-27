
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
    vector<Session> labs;
};

// Global course database
extern unordered_map<int, course> courseDB;

// Parses one "S,day,start,end,building,room"
Session parseSingleSession(const string& line);

// Splits "L S,1,... S,2,..." into multiple sessions
vector<Session> parseMultipleSessions(string line);

