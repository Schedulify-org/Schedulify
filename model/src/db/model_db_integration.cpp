#include "model_db_integration.h"

ModelDatabaseIntegration& ModelDatabaseIntegration::getInstance() {
    static ModelDatabaseIntegration instance;
    return instance;
}

bool ModelDatabaseIntegration::initializeDatabase(const string& dbPath) {
    QString qDbPath = dbPath.empty() ? QString() : QString::fromStdString(dbPath);

    // Check if already initialized
    if (m_initialized && DatabaseManager::getInstance().isConnected()) {
        return true;
    }

    // Try to initialize database with comprehensive error checking
    try {
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

        // Set up initial metadata
        updateLastAccessMetadata();

        Logger::get().logInfo("Database integration initialized successfully");

        return true;

    } catch (const exception& e) {
        Logger::get().logError("Exception during database initialization: " + string(e.what()));
        m_initialized = false;
        return false;
    }
}

bool ModelDatabaseIntegration::isInitialized() const {
    return m_initialized && DatabaseManager::getInstance().isConnected();
}

bool ModelDatabaseIntegration::loadCoursesToDatabase(const vector<Course>& courses, const string& fileName,
                                                     const string& fileType) {
    if (!isInitialized()) {
        Logger::get().logError("Database not initialized for course loading");
        return false;
    }

    if (courses.empty()) {
        Logger::get().logWarning("No courses provided to load into database");
        return true;
    }

    if (fileName.empty()) {
        Logger::get().logError("File name is required for course loading");
        return false;
    }

    if (fileType.empty()) {
        Logger::get().logError("File type is required for course loading");
        return false;
    }

    try {
        auto& db = DatabaseManager::getInstance();

        // Verify database connection before proceeding
        if (!db.isConnected()) {
            Logger::get().logError("Database connection lost during course loading");
            return false;
        }

        // create a new file entry for each upload
        int fileId = db.files()->insertFile(fileName, fileType);

        if (fileId <= 0) {
            Logger::get().logError("Failed to create file entry for: " + fileName);
            return false;
        }

        if (!db.courses()->insertCourses(courses, fileId)) {
            Logger::get().logError("Failed to insert courses into database for file ID: " + to_string(fileId));
            Logger::get().logWarning("File entry created but courses not saved - partial database state");
            return false;
        }

        // Update metadata
        db.updateMetadata("courses_loaded_at", QDateTime::currentDateTime().toString(Qt::ISODate).toStdString());
        db.updateMetadata("courses_count", to_string(courses.size()));
        db.updateMetadata("last_loaded_file", fileName);
        db.updateMetadata("last_file_type", fileType);

        updateLastAccessMetadata();

        Logger::get().logInfo("SUCCESS: All data saved to database");
        Logger::get().logInfo("File ID: " + to_string(fileId) + ", Courses: " + to_string(courses.size()));

        // Trigger callback if set
        if (m_onCoursesLoaded) {
            m_onCoursesLoaded(courses);
        }

        return true;

    } catch (const exception& e) {
        Logger::get().logError("Exception during course loading: " + string(e.what()));
        return false;
    }
}

vector<Course> ModelDatabaseIntegration::getCoursesByFileIds(const vector<int>& fileIds, vector<string>& warnings) {
    if (!isInitialized()) {
        Logger::get().logError("Database not initialized for course retrieval by file IDs");
        return {};
    }

    if (fileIds.empty()) {
        Logger::get().logWarning("No file IDs provided for course retrieval");
        return {};
    }

    // Handles conflicts by selecting the course from the latest uploaded file
    auto courses = DatabaseManager::getInstance().courses()->getCoursesByFileIds(fileIds, warnings);
    updateLastAccessMetadata();

    // Log conflict warnings that were generated
    if (!warnings.empty()) {
        Logger::get().logWarning("Resolved " + to_string(warnings.size()) + " course conflicts");
        for (const string& warning : warnings) {
            Logger::get().logWarning("CONFLICT: " + warning);
        }
    }

    return courses;
}

bool ModelDatabaseIntegration::insertFile(const string& fileName, const string& fileType) {
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
    Logger::get().logInfo("Successfully inserted file: " + fileName + " with ID: " + to_string(fileId));
    return true;
}

bool ModelDatabaseIntegration::updateFile(int fileId, const string& fileName, const string& fileType) {
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

        Logger::get().logInfo("Retrieved " + to_string(files.size()) + " files from database");

        // Log file details for debugging
        if (files.empty()) {
            Logger::get().logInfo("No files found in database - this is normal for first use");
        }
        return files;

    } catch (const exception& e) {
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

    Logger::get().logInfo("Retrieved " + to_string(courses.size()) + " courses from database");

    return courses;
}

bool ModelDatabaseIntegration::clearAllDatabaseData() {
    if (!isInitialized()) {
        Logger::get().logError("Database not initialized for clearing");
        return false;
    }

    if (!DatabaseManager::getInstance().clearAllData()) {
        Logger::get().logError("Failed to clear database data");
        return false;
    }

    // Re-initialize basic metadata
    auto& db = DatabaseManager::getInstance();
    db.insertMetadata("schema_version", to_string(DatabaseManager::getCurrentSchemaVersion()), "Database schema version");
    db.insertMetadata("created_at", QDateTime::currentDateTime().toString(Qt::ISODate).toStdString(), "Database creation timestamp");
    updateLastAccessMetadata();

    Logger::get().logInfo("Database data cleared successfully");
    return true;
}

ModelDatabaseIntegration::DatabaseStats ModelDatabaseIntegration::getDatabaseStats() {
    DatabaseStats stats = {};

    if (!isInitialized()) {
        return stats;
    }

    auto& db = DatabaseManager::getInstance();

    stats.courseCount = db.getTableRowCount("course");
    stats.fileCount = db.getTableRowCount("file");
    stats.metadataCount = db.getTableRowCount("metadata");

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

void ModelDatabaseIntegration::setOnCoursesLoaded(function<void(const vector<Course>&)> callback) {
    m_onCoursesLoaded = callback;
}

void ModelDatabaseIntegration::updateLastAccessMetadata() {
    if (isInitialized()) {
        auto& db = DatabaseManager::getInstance();
        db.updateMetadata("last_access", QDateTime::currentDateTime().toString(Qt::ISODate).toStdString());
    }
}