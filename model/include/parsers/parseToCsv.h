#ifndef PARSE_TO_XLSX_H
#define PARSE_TO_XLSX_H

#include "main_include.h"

#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <map>
#include <set>
#include <algorithm>
#include <sstream>
#include <iomanip>

int getHourFromTimeString(const std::string& timeStr);
void saveScheduleToCsv(const std::string& filePath, const InformativeSchedule& schedule);


#endif //PARSE_TO_XLSX_H
