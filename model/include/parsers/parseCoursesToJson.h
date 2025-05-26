#ifndef PARSE_COURSES_TO_JSON
#define PARSE_COURSES_TO_JSON

#include "model_interfaces.h"
#include "ScheduleBuilder.h"
#include "logger.h"

#include <algorithm>
#include <iomanip>

// filter the valid courses out of the course vector
vector<Course> filterValidCourses(const vector<Course>& courses);

// convert course vector to json file
bool exportCoursesToJson(const vector<Course>& courses, const string& outputPath);

#endif //PARSE_COURSES_TO_JSON
