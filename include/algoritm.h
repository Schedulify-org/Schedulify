#ifndef ALGORITM_H
#define ALGORITM_H

#include <string>
#include <vector>
#include "parser.h"
#include "schedule_types.h"

extern vector<Schedule> allPossibleSchedules;

int timeToMinutes(const string& t);
bool isOverlap(const Session* s1, const Session* s2);
std::vector<CourseSelection> getValidCourseCombinations(int courseId,
    const vector<const Session*>& lectures,
    const vector<const Session*>& tutorials,
    const vector<const Session*>& labs);
bool noConflict(const vector<CourseSelection>& current, const CourseSelection& cs);
void backtrack(int index, vector<CourseSelection>& current,
               const vector<vector<CourseSelection>>& courseOptions);

#endif 