#ifndef SCHED_MAIN_APP_H
#define SCHED_MAIN_APP_H

#include "fileHandlers/preParser.h"
#include "fileHandlers/parseToJson.h"
#include "schedule_algorithm/ScheduleBuilder.h"

#include <string>
#include <iostream>

using std::string;
using std::cout;

// main function to load data file, generate schedules and export as json
int main_app(const string& inputPath, const string& modifiedOutputPath, const string& userInput);

#endif
