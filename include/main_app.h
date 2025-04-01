#ifndef SCHED_MAIN_APP_H
#define SCHED_MAIN_APP_H

#include "../include/preParser.h"
#include "../include/postParser.h"
#include "../include/ScheduleBuilder.h"

#include <string>
#include <iostream>

using std::string;
using std::cout;

int main_app(const string& inputPath, const string& outputPath);
void printCourses(const vector<Course>& courses);
void printSchedules(const vector<Course>& courses);

#endif
