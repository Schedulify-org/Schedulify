#ifndef SCHED_MAIN_APP_H
#define SCHED_MAIN_APP_H

#include "parsers/preParser.h"
#include "parsers/postParser.h"
#include "algorithm/ScheduleBuilder.h"

#include <string>
#include <iostream>

using std::string;
using std::cout;

// main function to load data file, generate schedules and export as json
int main_app(const string& inputPath, const string& outputPath);

// print parsed data as test
void printCourses(const vector<Course>& courses);

// print schedules as test
void printSchedules(const vector<Course>& courses);

#endif
