#ifndef SCHED_MAIN_APP_H
#define SCHED_MAIN_APP_H

#include "../include/parser.h"
#include "../include/algoritm.h"

#include <string>
#include <iostream>

using std::string;
using std::cout;

int main_app(const string& inputPath, const string& outputPath);
void printCourses(const vector<Course>& courses);

#endif
