#ifndef PARSE_TO_XLSX_H
#define PARSE_TO_XLSX_H

#include "model_interfaces.h"
#include "logger/logger.h"

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


bool saveScheduleToCsv(const std::string& filePath, const InformativeSchedule& schedule);


#endif //PARSE_TO_XLSX_H
