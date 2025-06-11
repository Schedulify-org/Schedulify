#ifndef PARSE_TO_XLSX_H
#define PARSE_TO_XLSX_H

#include "model_interfaces.h"
#include "logger.h"

#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <map>
#include <set>
#include <algorithm>
#include <sstream>
#include <iomanip>

// Helper function to convert time string to hour integer
int getHourFromTimeString(const std::string& timeStr);

// Function to detect if text contains Hebrew characters
bool containsHebrew(const std::string& text);

// Function to get Hebrew translations for terms
std::string getHebrewTranslation(const std::string& englishTerm);

// Function to get Hebrew day names
std::string getHebrewDayName(const std::string& englishDay);

// Function to check if schedule contains Hebrew content
bool scheduleContainsHebrew(const InformativeSchedule& schedule);

// Main function to save schedule to CSV with Hebrew/RTL support
bool saveScheduleToCsv(const std::string& filePath, const InformativeSchedule& schedule);

#endif //PARSE_TO_XLSX_H