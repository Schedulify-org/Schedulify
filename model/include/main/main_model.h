#ifndef MAIN_MODEL_H
#define MAIN_MODEL_H

#include "model_interfaces.h"
#include "main/model_factory.h"
#include "schedule_algorithm/ScheduleBuilder.h"
#include "parsers/parseCoursesToVector.h"
#include "parsers/ScheduleEnrichment.h"
#include "parsers/printSchedule.h"
#include "parsers/parseToCsv.h"
#include "logger/logger.h"

#include <vector>
#include <string>
#include <iostream>

using std::string;
using std::cout;
using std::vector;

class Model : public IModel {
public:
    void* executeOperation(ModelOperation operation, const void* data, const string& path) override;

private:
    static vector<Course> generateCourses(const string& path);
    static vector<InformativeSchedule> generateSchedules(const vector<Course>& userInput, const vector<Course>& allCourses);
    static void saveSchedule(const InformativeSchedule& infoSchedule, const string& path);
    static void printSchedule(const InformativeSchedule& infoSchedule);

    vector<Course> lastGeneratedCourses;
    vector<InformativeSchedule> lastGeneratedSchedules;
};

#endif //MAIN_MODEL_H
