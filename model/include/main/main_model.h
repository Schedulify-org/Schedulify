#ifndef MAIN_MODEL_H
#define MAIN_MODEL_H

#include "model_interfaces.h"
#include "excel_parser.h"
#include "model_access.h"
#include "ScheduleBuilder.h"
#include "parseCoursesToVector.h"
#include "printSchedule.h"
#include "parseToCsv.h"
#include "logger.h"
#include "excel_parser.h"
#include "validate_courses.h"
#include "jsonParser.h"
#include "botAdapter.h"
#include "scheduleToMT.h"
#include "model_db_integration.h"
#include "db_manager.h"

#include <algorithm>
#include <cctype>
#include <vector>
#include <string>
#include <iostream>
#include <map>
#include <QDateTime>
#include <QSqlQuery>

using std::string;
using std::cout;
using std::vector;
using std::map;

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
    static vector<Course> loadCoursesFromDB();
    static vector<Course> loadCoursesFromHistory(const vector<int>& fileIds);
    static vector<FileEntity> getFileHistory();
    static vector<string> validateCourses(const vector<Course>& courses);
    static vector<InformativeSchedule> generateSchedules(const vector<Course>& userInput);
    static void saveSchedule(const InformativeSchedule& infoSchedule, const string& path);
    static void printSchedule(const InformativeSchedule& infoSchedule);
    static vector<string> messageBot(const vector<string>& userInput, const string& data);
    static bool deleteFileFromHistory(int fileId);
    static bool saveSchedulesToDB(const vector<InformativeSchedule>& schedules, const string& setName, const vector<int>& sourceFileIds);
    static vector<InformativeSchedule> loadSchedulesFromDB(int setId = -1);
    static vector<ScheduleSetEntity> getScheduleSetsFromDB();
    static bool deleteScheduleSetFromDB(int setId);

    vector<Course> lastGeneratedCourses;
    vector<string> courseFileErrors;
    vector<InformativeSchedule> lastGeneratedSchedules;
    string scheduleMetaData = "";
};

inline IModel* getModel() {
    return &Model::getInstance();
}

#endif //MAIN_MODEL_H