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
#include "scheduleToMT.h"
#include "model_db_integration.h"
#include "db_manager.h"
#include "sql_validator.h"
#include "claude_api_integration.h"
#include "schedule_filter_service.h"

#include <algorithm>
#include <cctype>
#include <vector>
#include <string>
#include <iostream>
#include <map>
#include <set>
#include <QDateTime>
#include <QSqlQuery>

using std::string;
using std::cout;
using std::vector;
using std::map;
using std::set;

class Model : public IModel {
public:
    static Model& getInstance() {
        static Model instance;
        return instance;
    }
    void* executeOperation(ModelOperation operation, const void* data, const string& path) override;
    Model(const Model&) = delete;
    Model& operator=(const Model&) = delete;

    // Structure for bot filter results
    struct BotFilterResult {
        std::vector<int> filteredScheduleIds;
        std::string responseMessage;
        bool hasError;
        std::string errorMessage;
        bool isFilterQuery;

        BotFilterResult() : hasError(false), isFilterQuery(false) {}
    };

private:
    Model() {}

    // Core business logic methods - these belong in main model
    static vector<Course> generateCourses(const string& path);
    static vector<Course> loadCoursesFromDB();
    static vector<Course> loadCoursesFromHistory(const vector<int>& fileIds);
    static vector<FileEntity> getFileHistory();
    static vector<string> validateCourses(const vector<Course>& courses);
    static vector<InformativeSchedule> generateSchedules(const vector<Course>& userInput);
    static void saveSchedule(const InformativeSchedule& infoSchedule, const string& path);
    static void printSchedule(const InformativeSchedule& infoSchedule);
    static bool deleteFileFromHistory(int fileId);
    static bool saveSchedulesToDB(const vector<InformativeSchedule>& schedules, const string& setName, const vector<int>& sourceFileIds);
    static vector<InformativeSchedule> loadSchedulesFromDB(int setId = -1);
    static vector<ScheduleSetEntity> getScheduleSetsFromDB();
    static bool deleteScheduleSetFromDB(int setId);
    static BotQueryResponse generateDemoResponse(const BotQueryRequest& request);
    static BotQueryResponse processClaudeQuery(const BotQueryRequest& request);

    // Bot query processing - main coordination method
    static BotFilterResult processBotQuery(const BotQueryRequest& request);

    // Static member to store last filtered IDs
    static std::vector<int> lastFilteredScheduleIds;

    vector<Course> lastGeneratedCourses;
    vector<string> courseFileErrors;
    vector<InformativeSchedule> lastGeneratedSchedules;
    string scheduleMetaData = "";
};

inline IModel* getModel() {
    return &Model::getInstance();
}

#endif //MAIN_MODEL_H