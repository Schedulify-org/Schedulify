#include "model_db_integration.h"

ModelDatabaseIntegration& ModelDatabaseIntegration::getInstance() {
    static ModelDatabaseIntegration instance;
    return instance;
}

bool ModelDatabaseIntegration::initializeDatabase(const string& dbPath) {
    Logger::get().logInfo("=== INITIALIZING DATABASE ===");

    QString qDbPath = dbPath.empty() ? QString() : QString::fromStdString(dbPath);

    // Check if already initialized
    if (m_initialized && DatabaseManager::getInstance().isConnected()) {
        Logger::get().logInfo("Database already initialized and connected");
        return true;
    }

    // Try to initialize database with comprehensive error checking
    try {
        Logger::get().logInfo("Attempting database connection...");

        if (!DatabaseManager::getInstance().initializeDatabase(qDbPath)) {
            Logger::get().logError("CRITICAL: DatabaseManager initialization failed");

            // Try to diagnose the issue
            Logger::get().logInfo("=== DATABASE DIAGNOSTIC ===");

            // Check if SQLite driver is available
            QStringList drivers = QSqlDatabase::drivers();
            Logger::get().logInfo("Available SQL drivers: " + drivers.join(", ").toStdString());

            if (!drivers.contains("QSQLITE")) {
                Logger::get().logError("QSQLITE driver not available - this is a critical issue");
                return false;
            }

            // Check directory permissions
            QString appDataPath = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
            Logger::get().logInfo("App data path: " + appDataPath.toStdString());

            QDir appDir(appDataPath);
            if (!appDir.exists()) {
                Logger::get().logInfo("Creating app data directory...");
                if (!appDir.mkpath(appDataPath)) {
                    Logger::get().logError("Cannot create app data directory - permission issue");
                    return false;
                }
            }

            return false;
        }

        m_initialized = true;
        Logger::get().logInfo("Database manager initialized successfully");

        // Set up initial metadata
        updateLastAccessMetadata();

        // Get and log database stats
        auto stats = getDatabaseStats();
        Logger::get().logInfo("Database integration initialized successfully");
        Logger::get().logInfo("Stats: " + std::to_string(stats.courseCount) + " courses, " +
                              std::to_string(stats.scheduleCount) + " schedules, " +
                              std::to_string(stats.metadataCount) + " files");

        return true;

    } catch (const std::exception& e) {
        Logger::get().logError("Exception during database initialization: " + string(e.what()));
        m_initialized = false;
        return false;
    }
}

bool ModelDatabaseIntegration::isInitialized() const {
    return m_initialized && DatabaseManager::getInstance().isConnected();
}

bool ModelDatabaseIntegration::loadCoursesToDatabase(const vector<Course>& courses, const string& fileName,
                                                     const string& fileType, const string& lut) {
    Logger::get().logInfo("=== LOADING COURSES TO DATABASE ===");

    if (!isInitialized()) {
        Logger::get().logError("Database not initialized for course loading");
        return false;
    }

    if (courses.empty()) {
        Logger::get().logWarning("No courses provided to load into database");
        return true; // Not an error - just nothing to do
    }

    if (fileName.empty()) {
        Logger::get().logError("File name is required for course loading");
        return false;
    }

    if (fileType.empty()) {
        Logger::get().logError("File type is required for course loading");
        return false;
    }

    Logger::get().logInfo("Loading " + std::to_string(courses.size()) + " courses with file: " + fileName);

    try {
        auto& db = DatabaseManager::getInstance();

        // Verify database connection before proceeding
        if (!db.isConnected()) {
            Logger::get().logError("Database connection lost during course loading");
            return false;
        }

        // REQUIREMENT 1: Always create a new file entry for each upload
        Logger::get().logInfo("Creating new file entry...");
        int fileId = db.files()->insertFile(fileName, fileType);

        if (fileId <= 0) {
            Logger::get().logError("Failed to create file entry for: " + fileName);

            // Try to diagnose the file insertion failure
            Logger::get().logInfo("=== FILE INSERTION DIAGNOSTIC ===");

            // Check if database is writable
            auto testFiles = db.files()->getAllFiles();
            Logger::get().logInfo("Current file count in database: " + std::to_string(testFiles.size()));

            // Check if it's a permission or constraint issue
            Logger::get().logError("Possible causes: database permissions, disk space, or constraint violations");

            return false;
        }

        Logger::get().logInfo("Created new file entry with ID: " + std::to_string(fileId));

        // Log course details before insertion
        Logger::get().logInfo("=== COURSE INSERTION DEBUG ===");
        for (size_t i = 0; i < std::min(courses.size(), size_t(3)); ++i) {
            Logger::get().logInfo("Course " + std::to_string(i) + " to insert: ID=" + std::to_string(courses[i].id) +
                                  ", Raw ID=" + courses[i].raw_id + ", Name=" + courses[i].name +
                                  ", File ID=" + std::to_string(fileId));
        }

        // REQUIREMENT 2: Insert all courses with the new file_id
        Logger::get().logInfo("Inserting courses with file ID: " + std::to_string(fileId));

        if (!db.courses()->insertCourses(courses, fileId)) {
            Logger::get().logError("Failed to insert courses into database for file ID: " + std::to_string(fileId));

            // The file entry exists but courses failed - this is a partial failure
            Logger::get().logWarning("File entry created but courses not saved - partial database state");

            return false;
        }

        Logger::get().logInfo("Successfully inserted " + std::to_string(courses.size()) + " courses");

        // Verify insertion by checking what was actually saved
        Logger::get().logInfo("=== VERIFICATION CHECK ===");
        auto savedCourses = db.courses()->getCoursesByFileId(fileId);
        Logger::get().logInfo("Verification: " + std::to_string(savedCourses.size()) + " courses found for file ID " + std::to_string(fileId));

        // Update metadata
        db.updateMetadata("courses_loaded_at", QDateTime::currentDateTime().toString(Qt::ISODate).toStdString());
        db.updateMetadata("courses_count", std::to_string(courses.size()));
        db.updateMetadata("last_loaded_file", fileName);
        db.updateMetadata("last_file_type", fileType);

        updateLastAccessMetadata();

        Logger::get().logInfo("SUCCESS: All data saved to database");
        Logger::get().logInfo("File ID: " + std::to_string(fileId) + ", Courses: " + std::to_string(courses.size()));

        // Trigger callback if set
        if (m_onCoursesLoaded) {
            m_onCoursesLoaded(courses);
        }

        return true;

    } catch (const std::exception& e) {
        Logger::get().logError("Exception during course loading: " + string(e.what()));
        return false;
    }
}

// REQUIREMENT 3: Get courses by file IDs with conflict resolution
vector<Course> ModelDatabaseIntegration::getCoursesByFileIds(const vector<int>& fileIds, vector<string>& warnings) {
    if (!isInitialized()) {
        Logger::get().logError("Database not initialized for course retrieval by file IDs");
        return {};
    }

    if (fileIds.empty()) {
        Logger::get().logWarning("No file IDs provided for course retrieval");
        return {};
    }

    Logger::get().logInfo("=== COURSE RETRIEVAL BY FILE IDS ===");
    Logger::get().logInfo("Loading courses from " + std::to_string(fileIds.size()) + " selected file(s)");

    // Log the specific file IDs being requested
    string fileIdsList;
    for (size_t i = 0; i < fileIds.size(); ++i) {
        if (i > 0) fileIdsList += ", ";
        fileIdsList += std::to_string(fileIds[i]);
    }
    Logger::get().logInfo("Requested file IDs: [" + fileIdsList + "]");

    // REQUIREMENT 4: Use the database course manager's conflict resolution
    // This handles conflicts by selecting the course from the latest uploaded file
    auto courses = DatabaseManager::getInstance().courses()->getCoursesByFileIds(fileIds, warnings);
    updateLastAccessMetadata();

    Logger::get().logInfo("=== COURSE RETRIEVAL RESULTS ===");
    Logger::get().logInfo("Retrieved " + std::to_string(courses.size()) + " courses from " +
                          std::to_string(fileIds.size()) + " file(s)");

    // Log first few retrieved courses for debugging
    if (!courses.empty()) {
        Logger::get().logInfo("=== RETRIEVED COURSES DEBUG ===");
        for (size_t i = 0; i < std::min(courses.size(), size_t(5)); ++i) {
            Logger::get().logInfo("Retrieved course " + std::to_string(i) + ": ID=" + std::to_string(courses[i].id) +
                                  ", Raw ID=" + courses[i].raw_id + ", Name=" + courses[i].name);
        }
    }

    // Log any conflict warnings that were generated
    if (!warnings.empty()) {
        Logger::get().logWarning("Resolved " + std::to_string(warnings.size()) + " course conflicts");
        for (const string& warning : warnings) {
            Logger::get().logWarning("CONFLICT: " + warning);
        }
    }

    return courses;
}

bool ModelDatabaseIntegration::loadSchedulesToDatabase(const vector<InformativeSchedule>& schedules,
                                                       const vector<Course>& usedCourses) {
    if (!isInitialized()) {
        Logger::get().logError("Database not initialized for schedule loading");
        return false;
    }

    if (schedules.empty()) {
        Logger::get().logWarning("No schedules provided to load into database");
        return true;
    }

    Logger::get().logInfo("Loading " + std::to_string(schedules.size()) + " schedules to database");

    auto& db = DatabaseManager::getInstance();

    // Clear existing schedules and metadata
    if (!db.deleteAllSchedules()) {
        Logger::get().logError("Failed to clear existing schedules");
        return false;
    }

    // Extract course IDs for each schedule
    vector<vector<int>> allCourseIds;
    for (const auto& schedule : schedules) {
        vector<int> courseIds = extractCourseIdsFromSchedule(schedule, usedCourses);
        allCourseIds.push_back(courseIds);
    }

    return loadSchedulesToDatabase(schedules, allCourseIds);
}

bool ModelDatabaseIntegration::loadSchedulesToDatabase(const vector<InformativeSchedule>& schedules,
                                                       const vector<vector<int>>& courseIdsPerSchedule) {
    if (!isInitialized()) {
        Logger::get().logError("Database not initialized for schedule loading");
        return false;
    }

    if (schedules.size() != courseIdsPerSchedule.size()) {
        Logger::get().logError("Schedules and course IDs vectors size mismatch");
        return false;
    }

    auto& db = DatabaseManager::getInstance();

    // Insert schedules
    if (!db.insertSchedules(schedules, courseIdsPerSchedule)) {
        Logger::get().logError("Failed to insert schedules into database");
        return false;
    }

    // Create generation settings JSON
    string generationSettings = generateGenerationSettings(vector<Course>()); // Empty for now

    // Insert schedule metadata
    if (!db.insertScheduleMetadata(static_cast<int>(schedules.size()), generationSettings)) {
        Logger::get().logError("Failed to insert schedule metadata");
        return false;
    }

    // Update metadata
    db.updateMetadata("schedules_loaded_at", QDateTime::currentDateTime().toString(Qt::ISODate).toStdString());
    db.updateMetadata("schedules_count", std::to_string(schedules.size()));

    updateLastAccessMetadata();

    Logger::get().logInfo("Successfully loaded " + std::to_string(schedules.size()) + " schedules to database");

    // Trigger callback if set
    if (m_onSchedulesLoaded) {
        m_onSchedulesLoaded(schedules);
    }

    return true;
}

// File operations
bool ModelDatabaseIntegration::insertFile(const string& fileName, const string& fileType, const string& lut) {
    if (!isInitialized()) {
        Logger::get().logError("Database not initialized for file insertion");
        return false;
    }

    auto& db = DatabaseManager::getInstance();
    int fileId = db.files()->insertFile(fileName, fileType);
    if (fileId <= 0) {
        Logger::get().logError("Failed to insert file into database");
        return false;
    }

    updateLastAccessMetadata();
    Logger::get().logInfo("Successfully inserted file: " + fileName + " with ID: " + std::to_string(fileId));
    return true;
}

bool ModelDatabaseIntegration::updateFile(int fileId, const string& fileName, const string& fileType, const string& lut) {
    if (!isInitialized()) {
        Logger::get().logError("Database not initialized for file update");
        return false;
    }

    auto& db = DatabaseManager::getInstance();
    if (!db.files()->updateFile(fileId, fileName, fileType)) {
        Logger::get().logError("Failed to update file in database");
        return false;
    }

    updateLastAccessMetadata();
    Logger::get().logInfo("Successfully updated file: " + fileName);
    return true;
}

vector<FileEntity> ModelDatabaseIntegration::getAllFiles() {
    Logger::get().logInfo("=== RETRIEVING ALL FILES FROM DATABASE ===");

    if (!isInitialized()) {
        Logger::get().logError("Database not initialized for file retrieval");
        return {};
    }

    try {
        auto& db = DatabaseManager::getInstance();

        // Verify database connection
        if (!db.isConnected()) {
            Logger::get().logError("Database connection lost during file retrieval");
            return {};
        }

        auto files = db.files()->getAllFiles();
        updateLastAccessMetadata();

        Logger::get().logInfo("Retrieved " + std::to_string(files.size()) + " files from database");

        // Log file details for debugging
        if (files.empty()) {
            Logger::get().logInfo("No files found in database - this is normal for first use");
        } else {
            Logger::get().logInfo("=== FILE LIST ===");
            for (const auto& file : files) {
                Logger::get().logInfo("ID=" + std::to_string(file.id) + ", Name='" + file.file_name +
                                      "', Type=" + file.file_type + ", Uploaded=" +
                                      file.upload_time.toString("yyyy-MM-dd hh:mm:ss").toStdString());
            }
        }

        return files;

    } catch (const std::exception& e) {
        Logger::get().logError("Exception during file retrieval: " + string(e.what()));
        return {};
    }
}

FileEntity ModelDatabaseIntegration::getFileByName(const string& fileName) {
    if (!isInitialized()) {
        Logger::get().logError("Database not initialized for file retrieval");
        return {};
    }

    auto file = DatabaseManager::getInstance().files()->getFileByName(fileName);
    updateLastAccessMetadata();

    return file;
}

int ModelDatabaseIntegration::getFileIdByName(const string& fileName) {
    if (!isInitialized()) {
        Logger::get().logError("Database not initialized for file ID retrieval");
        return -1;
    }

    int fileId = DatabaseManager::getInstance().files()->getFileIdByName(fileName);
    updateLastAccessMetadata();

    return fileId;
}

vector<Course> ModelDatabaseIntegration::getCoursesFromDatabase() {
    if (!isInitialized()) {
        Logger::get().logError("Database not initialized for course retrieval");
        return {};
    }

    auto courses = DatabaseManager::getInstance().courses()->getAllCourses();
    updateLastAccessMetadata();

    Logger::get().logInfo("Retrieved " + std::to_string(courses.size()) + " courses from database");

    return courses;
}

vector<InformativeSchedule> ModelDatabaseIntegration::getSchedulesFromDatabase() {
    if (!isInitialized()) {
        Logger::get().logError("Database not initialized for schedule retrieval");
        return {};
    }

    auto schedules = DatabaseManager::getInstance().getAllSchedules();
    updateLastAccessMetadata();

    Logger::get().logInfo("Retrieved " + std::to_string(schedules.size()) + " schedules from database");

    return schedules;
}

bool ModelDatabaseIntegration::migrateExistingData(const vector<Course>& existingCourses,
                                                   const vector<InformativeSchedule>& existingSchedules,
                                                   const vector<Course>& usedCourses) {
    if (!isInitialized()) {
        Logger::get().logError("Database not initialized for migration");
        return false;
    }

    Logger::get().logInfo("Starting data migration: " +
                          std::to_string(existingCourses.size()) + " courses, " +
                          std::to_string(existingSchedules.size()) + " schedules");

    auto& db = DatabaseManager::getInstance();
    DatabaseTransaction transaction(db);

    try {
        // Load courses
        if (!existingCourses.empty()) {
            // Create a default migration file entry
            string migrationFileName = "migration_" + QDateTime::currentDateTime().toString("yyyyMMdd_hhmmss").toStdString();
            string migrationFileType = "migration";

            if (!loadCoursesToDatabase(existingCourses, migrationFileName, migrationFileType)) {
                Logger::get().logError("Failed to migrate courses");
                return false;
            }
        }

        // Load schedules
        if (!existingSchedules.empty()) {
            if (!loadSchedulesToDatabase(existingSchedules, usedCourses)) {
                Logger::get().logError("Failed to migrate schedules");
                return false;
            }
        }

        // Set migration metadata
        db.updateMetadata("migration_completed_at", QDateTime::currentDateTime().toString(Qt::ISODate).toStdString());
        db.updateMetadata("migration_version", "1.0");

        if (!transaction.commit()) {
            Logger::get().logError("Failed to commit migration transaction");
            return false;
        }

        Logger::get().logInfo("Data migration completed successfully");
        return true;

    } catch (const std::exception& e) {
        Logger::get().logError("Exception during migration: " + string(e.what()));
        return false;
    }
}

bool ModelDatabaseIntegration::clearAllDatabaseData() {
    if (!isInitialized()) {
        Logger::get().logError("Database not initialized for clearing");
        return false;
    }

    Logger::get().logInfo("Clearing all database data");

    if (!DatabaseManager::getInstance().clearAllData()) {
        Logger::get().logError("Failed to clear database data");
        return false;
    }

    // Re-initialize basic metadata
    auto& db = DatabaseManager::getInstance();
    db.insertMetadata("schema_version", std::to_string(DatabaseManager::getCurrentSchemaVersion()), "Database schema version");
    db.insertMetadata("created_at", QDateTime::currentDateTime().toString(Qt::ISODate).toStdString(), "Database creation timestamp");
    updateLastAccessMetadata();

    Logger::get().logInfo("Database data cleared successfully");
    return true;
}

bool ModelDatabaseIntegration::backupDatabase(const string& backupPath) {
    if (!isInitialized()) {
        Logger::get().logError("Database not initialized for backup");
        return false;
    }

    // TODO: Implement database backup functionality
    Logger::get().logWarning("Database backup functionality not yet implemented");
    return false;
}

bool ModelDatabaseIntegration::restoreDatabase(const string& backupPath) {
    if (!isInitialized()) {
        Logger::get().logError("Database not initialized for restore");
        return false;
    }

    // TODO: Implement database restore functionality
    Logger::get().logWarning("Database restore functionality not yet implemented");
    return false;
}

ModelDatabaseIntegration::DatabaseStats ModelDatabaseIntegration::getDatabaseStats() {
    DatabaseStats stats = {};

    if (!isInitialized()) {
        return stats;
    }

    auto& db = DatabaseManager::getInstance();

    stats.courseCount = db.getTableRowCount("course");
    stats.scheduleCount = db.getTableRowCount("schedule");
    stats.metadataCount = db.getTableRowCount("file"); // Changed to file count instead of metadata
    stats.scheduleMetadataCount = db.getTableRowCount("schedule_metadata");

    stats.lastUpdate = db.getMetadata("last_access", "Never");

    // Get database file size (approximate)
    stats.databaseSize = "Unknown";

    return stats;
}

void ModelDatabaseIntegration::enableAutoSave(bool enable) {
    m_autoSaveEnabled = enable;

    auto& db = DatabaseManager::getInstance();
    db.updateMetadata("auto_save_enabled", enable ? "true" : "false");

    Logger::get().logInfo("Auto-save " + string(enable ? "enabled" : "disabled"));
}

bool ModelDatabaseIntegration::isAutoSaveEnabled() const {
    return m_autoSaveEnabled;
}

void ModelDatabaseIntegration::setOnCoursesLoaded(std::function<void(const vector<Course>&)> callback) {
    m_onCoursesLoaded = callback;
}

void ModelDatabaseIntegration::setOnSchedulesLoaded(std::function<void(const vector<InformativeSchedule>&)> callback) {
    m_onSchedulesLoaded = callback;
}

// Helper methods
vector<int> ModelDatabaseIntegration::extractCourseIdsFromSchedule(const InformativeSchedule& schedule,
                                                                   const vector<Course>& availableCourses) {
    set<int> uniqueIds;

    // Extract course IDs from schedule items
    for (const auto& day : schedule.week) {
        for (const auto& item : day.day_items) {
            // Find course by raw_id
            auto it = std::find_if(availableCourses.begin(), availableCourses.end(),
                                   [&item](const Course& course) {
                                       return course.raw_id == item.raw_id;
                                   });

            if (it != availableCourses.end()) {
                uniqueIds.insert(it->id);
            }
        }
    }

    return vector<int>(uniqueIds.begin(), uniqueIds.end());
}

string ModelDatabaseIntegration::generateGenerationSettings(const vector<Course>& usedCourses) {
    QJsonObject settings;
    settings["timestamp"] = QDateTime::currentDateTime().toString(Qt::ISODate);
    settings["course_count"] = static_cast<int>(usedCourses.size());
    settings["generation_method"] = "automatic";
    settings["version"] = "1.0";

    QJsonArray courseIds;
    for (const auto& course : usedCourses) {
        courseIds.append(course.id);
    }
    settings["course_ids"] = courseIds;

    QJsonDocument doc(settings);
    return doc.toJson(QJsonDocument::Compact).toStdString();
}

void ModelDatabaseIntegration::updateLastAccessMetadata() {
    if (isInitialized()) {
        auto& db = DatabaseManager::getInstance();
        db.updateMetadata("last_access", QDateTime::currentDateTime().toString(Qt::ISODate).toStdString());
    }
}