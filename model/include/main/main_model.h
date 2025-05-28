#ifndef MAIN_MODEL_H
#define MAIN_MODEL_H

#include "model_interfaces.h"
#include "excel_parser.h"
#include "model_access.h"
#include "ScheduleBuilder.h"
#include "parseCoursesToVector.h"
#include "ScheduleEnrichment.h"
#include "printSchedule.h"
#include "parseToCsv.h"
#include "logger.h"

#include <vector>
#include <string>
#include <iostream>

using std::string;
using std::cout;
using std::vector;

class Model : public IModel {
public:
    static Model& getInstance() {
        static Model instance;
        return instance;
    }
    void* executeOperation(ModelOperation operation, const void* data, const string& path) override;
    Model(const Model&) = delete;
    Model& operator=(const Model&) = delete;

private:
    Model() {}
    static vector<Course> generateCourses(const string& path);
    static vector<InformativeSchedule> generateSchedules(const vector<Course>& userInput);
    static void saveSchedule(const InformativeSchedule& infoSchedule, const string& path);
    static void printSchedule(const InformativeSchedule& infoSchedule);

    vector<Course> lastGeneratedCourses;
    vector<InformativeSchedule> lastGeneratedSchedules;
};

inline IModel* getModel() {
    return &Model::getInstance();
}

#endif //MAIN_MODEL_H
