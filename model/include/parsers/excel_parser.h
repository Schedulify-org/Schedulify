#ifndef EXCEL_PARSER_H
#define EXCEL_PARSER_H

#include <OpenXLSX.hpp>
#include <iostream>
#include <vector>
#include <string>
#include <map>
#include <regex>
#include <unordered_map>
#include <sstream>
#include <algorithm>

#ifdef _WIN32
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#ifndef NOMINMAX
#define NOMINMAX
#endif
#define byte win_byte_override
#include <windows.h>
#include <io.h>
#include <fcntl.h>
#include <locale>
#undef byte
#undef WIN32_LEAN_AND_MEAN
#undef NOMINMAX
#endif

#include "model_interfaces.h"

using namespace std;
using namespace OpenXLSX;

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