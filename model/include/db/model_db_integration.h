#ifndef MODEL_DATABASE_INTEGRATION_H
#define MODEL_DATABASE_INTEGRATION_H

#include "model_interfaces.h"
#include "db_manager.h"
#include "logger.h"

#include <vector>
#include <string>
#include <functional>
#include <QFileInfo>
#include <QDir>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QDateTime>
#include <algorithm>
#include <set>

using namespace std;

class ModelDatabaseIntegration {
public:
    static ModelDatabaseIntegration& getInstance();

    // Initialization
    bool initializeDatabase(const string& dbPath = "");
    bool isInitialized() const;

    // Data loading operations
    bool loadCoursesToDatabase(const vector<Course>& courses, const string& fileName = "",
                               const string& fileType = "", const string& lut = "");
    bool loadSchedulesToDatabase(const vector<InformativeSchedule>& schedules,
                                 const vector<Course>& usedCourses);
    bool loadSchedulesToDatabase(const vector<InformativeSchedule>& schedules,
                                 const vector<vector<int>>& courseIdsPerSchedule);

    // File operations
    bool insertFile(const string& fileName, const string& fileType, const string& lut = "");
    bool updateFile(int fileId, const string& fileName, const string& fileType, const string& lut = "");
    vector<FileEntity> getAllFiles();
    FileEntity getFileByName(const string& fileName);
    int getFileIdByName(const string& fileName);

    // Data retrieval operations
    vector<Course> getCoursesFromDatabase();

    // REQUIREMENT 3: Get courses by file IDs with conflict resolution (REQUIREMENT 4)
    vector<Course> getCoursesByFileIds(const vector<int>& fileIds, vector<string>& warnings);

    vector<InformativeSchedule> getSchedulesFromDatabase();

    // Migration and maintenance
    bool migrateExistingData(const vector<Course>& existingCourses,
                             const vector<InformativeSchedule>& existingSchedules,
                             const vector<Course>& usedCourses);

    // Utility operations
    bool clearAllDatabaseData();
    bool backupDatabase(const string& backupPath);
    bool restoreDatabase(const string& backupPath);

    // Statistics and info
    struct DatabaseStats {
        int courseCount;
        int scheduleCount;
        int metadataCount;        // Now represents file count for clarity
        int scheduleMetadataCount;
        string databaseSize;
        string lastUpdate;
    };

    DatabaseStats getDatabaseStats();

    // Auto-save functionality
    void enableAutoSave(bool enable = true);
    bool isAutoSaveEnabled() const;

    // Event callbacks for data changes
    void setOnCoursesLoaded(std::function<void(const vector<Course>&)> callback);
    void setOnSchedulesLoaded(std::function<void(const vector<InformativeSchedule>&)> callback);

private:
    ModelDatabaseIntegration() = default;
    ~ModelDatabaseIntegration() = default;

    // Disable copy/move
    ModelDatabaseIntegration(const ModelDatabaseIntegration&) = delete;
    ModelDatabaseIntegration& operator=(const ModelDatabaseIntegration&) = delete;

    // Helper methods
    vector<int> extractCourseIdsFromSchedule(const InformativeSchedule& schedule,
                                             const vector<Course>& availableCourses);
    string generateGenerationSettings(const vector<Course>& usedCourses);
    void updateLastAccessMetadata();

    // State
    bool m_initialized = false;
    bool m_autoSaveEnabled = false;

    // Callbacks
    std::function<void(const vector<Course>&)> m_onCoursesLoaded;
    std::function<void(const vector<InformativeSchedule>&)> m_onSchedulesLoaded;
};

#endif // MODEL_DATABASE_INTEGRATION_H