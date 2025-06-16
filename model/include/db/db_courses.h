#ifndef DB_COURSES_H
#define DB_COURSES_H

#include "db_entities.h"
#include "model_interfaces.h"
#include "db_json_helpers.h"
#include "logger.h"

#include <QSqlQuery>
#include <QSqlError>
#include <QVariant>
#include <algorithm>
#include <QSqlDatabase>
#include <vector>
#include <string>
#include <map>

using namespace std;

class DatabaseCourseManager {
public:
    explicit DatabaseCourseManager(QSqlDatabase& database);

    // Course CRUD operations
    bool insertCourse(const Course& course, int fileId);
    bool insertCourses(const vector<Course>& courses, int fileId);
    bool updateCourse(const Course& course, int fileId);
    bool deleteCourse(int courseId);
    bool deleteAllCourses();
    bool deleteCoursesByFileId(int fileId);

    // Course retrieval operations
    vector<Course> getAllCourses();
    Course getCourseById(int id);
    vector<Course> getCoursesByName(const string& name);
    vector<Course> getCoursesByFileId(int fileId);
    vector<Course> getCoursesByFileIds(const vector<int>& fileIds, vector<string>& warnings);
    vector<Course> getCoursesByRawId(const string& rawId);

    // Course utility operations
    bool courseExists(int courseId);
    bool courseExistsByRawId(const string& rawId);
    bool courseExistsByRawIdAndFileId(const string& rawId, int fileId);
    int getCourseCount();
    int getCourseCountByFileId(int fileId);
    QDateTime getCourseCreationTime(int courseId);

    // Course statistics
    map<string, int> getCourseCountsByType();
    vector<Course> getCoursesCreatedAfter(const QDateTime& dateTime);
    vector<Course> getCoursesByTeacher(const string& teacher);

private:
    QSqlDatabase& db;

    // Helper methods
    Course createCourseFromQuery(QSqlQuery& query);

    // Conflict resolution helpers
    struct CourseConflictInfo {
        Course course;
        QDateTime uploadTime;
        string fileName;
        int fileId;
    };

    vector<Course> resolveConflicts(const map<string, vector<CourseConflictInfo>>& conflictMap,
                                    vector<string>& warnings);
};

#endif // DB_COURSES_H