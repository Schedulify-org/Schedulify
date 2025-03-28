#ifndef ALGORITM_H
#define ALGORITM_H

#include <string>
#include <vector>
#include "parser.h"

using namespace std;

struct CourseSelection {
    int courseId;
    const Session* lecture;
    const Session* tutorial; // nullptr if none
    const Session* lab;      // nullptr if none
};

struct Schedule {
    vector<CourseSelection> selections;
};

extern vector<Schedule> allPossibleSchedules;

int timeToMinutes(const string& t);
bool isOverlap(const Session* s1, const Session* s2);
vector<CourseSelection> getValidCourseCombinations(int courseId,
    const vector<const Session*>& lectures,
    const vector<const Session*>& tutorials,
    const vector<const Session*>& labs);
vector<const Session*> getSessions(const CourseSelection& cs);
bool noConflict(const vector<CourseSelection>& current, const CourseSelection& cs);
void backtrack(int index, vector<CourseSelection>& current,
               const vector<vector<CourseSelection>>& courseOptions);

#endif 