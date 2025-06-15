#ifndef DATABASE_MANAGER_H
#define DATABASE_MANAGER_H

#include "db_entities.h"
#include "db_schema.h"
#include "db_files.h"
#include "db_courses.h"
#include "db_schedules.h"
#include "model_interfaces.h"
#include "logger.h"

#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlError>
#include <QStandardPaths>
#include <QDir>
#include <QString>
#include <QVariant>
#include <QDateTime>
#include <vector>
#include <string>
#include <memory>

using namespace std;

// Forward declaration for friend class
class DatabaseRepair;

class DatabaseManager {
public:
    static DatabaseManager& getInstance();

    // Database lifecycle
    bool initializeDatabase(const QString& dbPath = QString());
    void debugDatabaseContents();
    bool isConnected() const;
    void closeDatabase();

    // Manager access - providing delegation to specialized managers
    DatabaseFileManager* files() { return fileManager.get(); }
    DatabaseCourseManager* courses() { return courseManager.get(); }
    DatabaseScheduleManager* schedules() { return scheduleManager.get(); }
    DatabaseSchema* schema() { return schemaManager.get(); }

    // Metadata operations (kept in main manager for simplicity)
    bool insertMetadata(const string& key, const string& value, const string& description = "");
    bool updateMetadata(const string& key, const string& value);
    string getMetadata(const string& key, const string& defaultValue = "");
    vector<MetadataEntity> getAllMetadata();

    // Convenience methods that delegate to schedule manager
    bool insertSchedule(const InformativeSchedule& schedule, const vector<int>& courseIds) {
        return scheduleManager->insertSchedule(schedule, courseIds);
    }
    bool insertSchedules(const vector<InformativeSchedule>& schedules,
                         const vector<vector<int>>& allCourseIds) {
        return scheduleManager->insertSchedules(schedules, allCourseIds);
    }
    bool deleteAllSchedules() {
        return scheduleManager->deleteAllSchedules();
    }
    vector<InformativeSchedule> getAllSchedules() {
        return scheduleManager->getAllSchedules();
    }
    InformativeSchedule getScheduleById(int id) {
        return scheduleManager->getScheduleById(id);
    }

    // Schedule metadata operations (delegate to schedule manager)
    bool insertScheduleMetadata(int totalSchedules, const string& generationSettings) {
        return scheduleManager->insertScheduleMetadata(totalSchedules, generationSettings);
    }
    bool updateScheduleMetadata(int id, const string& status) {
        return scheduleManager->updateScheduleMetadata(id, status);
    }
    vector<ScheduleMetadataEntity> getAllScheduleMetadata() {
        return scheduleManager->getAllScheduleMetadata();
    }
    ScheduleMetadataEntity getLatestScheduleMetadata() {
        return scheduleManager->getLatestScheduleMetadata();
    }

    // Utility operations
    bool clearAllData();
    bool exportToFile(const QString& filePath);
    bool importFromFile(const QString& filePath);
    int getTableRowCount(const string& tableName);

    // Database repair and debugging
    bool repairDatabase();
    void debugDatabaseSchema();

    // Transaction support
    bool beginTransaction();
    bool commitTransaction();
    bool rollbackTransaction();

    // Schema version access
    static int getCurrentSchemaVersion() { return CURRENT_SCHEMA_VERSION; }

    // Backward compatibility methods (delegate to specialized managers)
    // File operations
    int insertFile(const string& fileName, const string& fileType) {
        return fileManager->insertFile(fileName, fileType);
    }
    bool updateFile(int fileId, const string& fileName, const string& fileType) {
        return fileManager->updateFile(fileId, fileName, fileType);
    }
    bool deleteFile(int fileId) {
        return fileManager->deleteFile(fileId);
    }
    bool deleteAllFiles() {
        return fileManager->deleteAllFiles();
    }
    vector<FileEntity> getAllFiles() {
        return fileManager->getAllFiles();
    }
    FileEntity getFileById(int id) {
        return fileManager->getFileById(id);
    }
    FileEntity getFileByName(const string& fileName) {
        return fileManager->getFileByName(fileName);
    }
    int getFileIdByName(const string& fileName) {
        return fileManager->getFileIdByName(fileName);
    }
    QDateTime getFileUploadTime(int fileId) {
        return fileManager->getFileUploadTime(fileId);
    }

    // Course operations
    bool insertCourse(const Course& course, int fileId) {
        return courseManager->insertCourse(course, fileId);
    }
    bool insertCourses(const vector<Course>& courses, int fileId) {
        return courseManager->insertCourses(courses, fileId);
    }
    bool updateCourse(const Course& course, int fileId) {
        return courseManager->updateCourse(course, fileId);
    }
    bool deleteCourse(int courseId) {
        return courseManager->deleteCourse(courseId);
    }
    bool deleteAllCourses() {
        return courseManager->deleteAllCourses();
    }
    bool deleteCoursesByFileId(int fileId) {
        return courseManager->deleteCoursesByFileId(fileId);
    }
    vector<Course> getAllCourses() {
        return courseManager->getAllCourses();
    }
    Course getCourseById(int id) {
        return courseManager->getCourseById(id);
    }
    vector<Course> getCoursesByName(const string& name) {
        return courseManager->getCoursesByName(name);
    }
    vector<Course> getCoursesByFileId(int fileId) {
        return courseManager->getCoursesByFileId(fileId);
    }
    vector<Course> getCoursesByFileIds(const vector<int>& fileIds, vector<string>& warnings) {
        return courseManager->getCoursesByFileIds(fileIds, warnings);
    }
    bool courseExistsByRawIdAndFileId(const string& rawId, int fileId) {
        return courseManager->courseExistsByRawIdAndFileId(rawId, fileId);
    }
    QDateTime getCourseCreationTime(int courseId) {
        return courseManager->getCourseCreationTime(courseId);
    }

private:
    // Allow DatabaseRepair class to access private members
    friend class DatabaseRepair;

    DatabaseManager() = default;
    ~DatabaseManager();

    // Disable copy/move
    DatabaseManager(const DatabaseManager&) = delete;
    DatabaseManager& operator=(const DatabaseManager&) = delete;
    DatabaseManager(DatabaseManager&&) = delete;
    DatabaseManager& operator=(DatabaseManager&&) = delete;

    // Internal methods
    bool executeQuery(const QString& query, const QVariantList& params = QVariantList());
    QSqlQuery prepareQuery(const QString& query);

    // Database connection and managers
    QSqlDatabase db;
    bool isInitialized = false;

    // Specialized managers
    std::unique_ptr<DatabaseSchema> schemaManager;
    std::unique_ptr<DatabaseFileManager> fileManager;
    std::unique_ptr<DatabaseCourseManager> courseManager;
    std::unique_ptr<DatabaseScheduleManager> scheduleManager;

    // Database schema version for migrations
    static const int CURRENT_SCHEMA_VERSION = 3;  // Updated to version 3
};

class DatabaseTransaction {
public:
    explicit DatabaseTransaction(DatabaseManager& dbManager);
    ~DatabaseTransaction();

    bool commit();
    void rollback();

private:
    DatabaseManager& db;
    bool committed = false;
    bool rolledBack = false;
};

#endif // DATABASE_MANAGER_H