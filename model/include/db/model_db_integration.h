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
#include <QDateTime>

using namespace std;

class ModelDatabaseIntegration {
public:
    static ModelDatabaseIntegration& getInstance();

    // Initialization
    bool initializeDatabase(const string& dbPath = "");
    bool isInitialized() const;

    // Data loading operations
    bool loadCoursesToDatabase(const vector<Course>& courses, const string& fileName = "", const string& fileType = "");

    // File operations
    bool insertFile(const string& fileName, const string& fileType);
    bool updateFile(int fileId, const string& fileName, const string& fileType);
    vector<FileEntity> getAllFiles();
    FileEntity getFileByName(const string& fileName);
    int getFileIdByName(const string& fileName);

    // Data retrieval operations
    vector<Course> getCoursesFromDatabase();

    // Get courses by file IDs with conflict resolution
    vector<Course> getCoursesByFileIds(const vector<int>& fileIds, vector<string>& warnings);

    // Utility operations
    bool clearAllDatabaseData();

    // Statistics and info
    struct DatabaseStats {
        int courseCount;
        int fileCount;
        int metadataCount;
        string databaseSize;
        string lastUpdate;
    };

    DatabaseStats getDatabaseStats();

    // Auto-save functionality
    void enableAutoSave(bool enable = true);
    bool isAutoSaveEnabled() const;

    // Event callbacks for data changes
    void setOnCoursesLoaded(std::function<void(const vector<Course>&)> callback);

    bool saveSchedulesToDatabase(const vector<InformativeSchedule>& schedules, const string& setName = "",
                                 const vector<int>& sourceFileIds = {});
    vector<InformativeSchedule> getSchedulesFromDatabase(int setId = -1);
    vector<ScheduleSetEntity> getScheduleSets();
    bool deleteScheduleSet(int setId);
    vector<InformativeSchedule> filterSchedulesByMetrics(const ScheduleFilterData& filters);


private:
    ModelDatabaseIntegration() = default;
    ~ModelDatabaseIntegration() = default;

    // Helper methods
    void updateLastAccessMetadata();

    // State
    bool m_initialized = false;
    bool m_autoSaveEnabled = false;

    // Callbacks
    std::function<void(const vector<Course>&)> m_onCoursesLoaded;
};

#endif // MODEL_DATABASE_INTEGRATION_H