#include "main_model.h"

std::string getFileExtension(const std::string& filename) {
    size_t dot = filename.find_last_of(".");
    if (dot == std::string::npos) {
        return "";
    }
    std::string ext = filename.substr(dot + 1);
    std::transform(ext.begin(), ext.end(), ext.begin(), ::tolower);
    return ext;
}

vector<Course> Model::generateCourses(const string& path) {
    vector<Course> courses;

    // Start collecting logger messages for parsing
    Logger::get().startCollecting();

    try {
        auto& dbIntegration = ModelDatabaseIntegration::getInstance();
        if (!dbIntegration.isInitialized()) {
            if (!dbIntegration.initializeDatabase()) {
                Logger::get().logError("Failed to initialize database - proceeding without persistence");
            } else {
                Logger::get().logInfo("Database initialized successfully");
            }
        }

        std::string extension = getFileExtension(path);

        if (extension == "xlsx") {
            Logger::get().logInfo("Parsing Excel file: " + path);
            ExcelCourseParser excelParser;
            courses = excelParser.parseExcelFile(path);
        }
        else if (extension == "txt") {
            Logger::get().logInfo("Parsing text file: " + path);
            courses = parseCourseDB(path);
        }
        else {
            Logger::get().logError("Unsupported file format: " + extension + ". Supported formats: .txt, .xlsx");
            Logger::get().stopCollecting();
            return courses; // Return empty vector
        }

        if (courses.empty()) {
            Logger::get().logError("Error while parsing input data from file: " + path + ". No courses found.");
        } else {
            Logger::get().logInfo("Successfully parsed " + std::to_string(courses.size()) + " courses from " + path);

            // Extract file information from path
            size_t lastSlash = path.find_last_of("/\\");
            string fileName = (lastSlash != string::npos) ? path.substr(lastSlash + 1) : path;
            string fileType = extension;

            if (dbIntegration.isInitialized()) {
                try {
                    if (dbIntegration.loadCoursesToDatabase(courses, fileName, fileType)) {
                        Logger::get().logInfo("SUCCESS: Courses and file metadata saved to database");
                        Logger::get().logInfo("- File: " + fileName + " (type: " + fileType + ")");
                        Logger::get().logInfo("- Courses: " + std::to_string(courses.size()) + " courses linked to file");
                    } else {
                        Logger::get().logWarning("Failed to load courses into database, continuing without persistence");
                    }
                } catch (const std::exception& e) {
                    Logger::get().logWarning("Database error while loading courses: " + string(e.what()));
                    Logger::get().logWarning("Continuing without database persistence");
                }
            } else {
                Logger::get().logWarning("Database not initialized - continuing without persistence");
            }
        }

    } catch (const std::exception& e) {
        Logger::get().logError("Exception during parsing: " + string(e.what()));
    }

    Logger::get().logInfo(std::to_string(courses.size()) + " courses loaded");

    // Keep collecting enabled for validation phase
    return courses;
}

vector<Course> Model::loadCoursesFromDB() {
    auto& dbIntegration = ModelDatabaseIntegration::getInstance();
    if (!dbIntegration.isInitialized()) {
        dbIntegration.initializeDatabase();
    }
    return dbIntegration.getCoursesFromDatabase();
}

vector<Course> Model::loadCoursesFromHistory(const vector<int>& fileIds) {
    vector<Course> courses;
    vector<string> warnings;

    Logger::get().startCollecting();

    try {
        if (fileIds.empty()) {
            Logger::get().logError("No file IDs provided for loading from history");
            Logger::get().stopCollecting();
            return courses;
        }

        Logger::get().logInfo("=== LOADING COURSES FROM HISTORY ===");
        Logger::get().logInfo("Requested " + std::to_string(fileIds.size()) + " file(s)");

        auto& dbIntegration = ModelDatabaseIntegration::getInstance();
        if (!dbIntegration.isInitialized()) {
            Logger::get().logInfo("Initializing database for history loading");
            if (!dbIntegration.initializeDatabase()) {
                Logger::get().logError("Failed to initialize database for history loading");
                Logger::get().stopCollecting();
                return courses;
            }
        }

        // Log file IDs being requested
        string fileIdsList;
        for (size_t i = 0; i < fileIds.size(); ++i) {
            if (i > 0) fileIdsList += ", ";
            fileIdsList += std::to_string(fileIds[i]);
        }
        Logger::get().logInfo("Requested file IDs: [" + fileIdsList + "]");

        // Get courses by file IDs with conflict resolution
        courses = dbIntegration.getCoursesByFileIds(fileIds, warnings);

        Logger::get().logInfo("=== HISTORY LOADING RESULTS ===");
        Logger::get().logInfo("File IDs requested: [" + fileIdsList + "]");
        Logger::get().logInfo("Courses loaded: " + std::to_string(courses.size()));
        Logger::get().logInfo("Conflicts resolved: " + std::to_string(warnings.size()));

        // Log conflict resolution warnings
        if (!warnings.empty()) {
            Logger::get().logWarning("=== CONFLICT RESOLUTION ===");
            for (const string& warning : warnings) {
                Logger::get().logWarning(warning);
            }
        }

        // Log first few loaded courses for debugging
        if (!courses.empty()) {
            Logger::get().logInfo("=== LOADED COURSES DEBUG ===");
            for (size_t i = 0; i < std::min(courses.size(), size_t(5)); ++i) {
                Logger::get().logInfo("Course " + std::to_string(i) + ": ID=" + std::to_string(courses[i].id) +
                                      ", Raw ID=" + courses[i].raw_id + ", Name=" + courses[i].name);
            }
        }

        // Enhanced debugging for empty results
        if (courses.empty()) {
            Logger::get().logWarning("=== NO COURSES FOUND - DEBUGGING ===");

            // Check database connection
            auto& db = DatabaseManager::getInstance();
            if (!db.isConnected()) {
                Logger::get().logError("Database is not connected!");
                return courses;
            }

            // Check if files exist in database
            for (int fileId : fileIds) {
                FileEntity file = db.files()->getFileById(fileId);
                if (file.id != 0) {
                    Logger::get().logInfo("File ID " + std::to_string(fileId) + " exists: '" + file.file_name + "'");

                    // Check courses for this file
                    vector<Course> fileCourses = db.courses()->getCoursesByFileId(fileId);
                    Logger::get().logInfo("File " + std::to_string(fileId) + " has " + std::to_string(fileCourses.size()) + " courses");

                    if (fileCourses.empty()) {
                        Logger::get().logWarning("File exists but has no associated courses - possible data corruption");
                    }
                } else {
                    Logger::get().logError("File ID " + std::to_string(fileId) + " not found in database");
                }
            }
        }

    } catch (const std::exception& e) {
        Logger::get().logError("Exception during loading from history: " + string(e.what()));
        courses.clear();
    }

    return courses;
}

vector<FileEntity> Model::getFileHistory() {
    try {
        auto& dbIntegration = ModelDatabaseIntegration::getInstance();
        if (!dbIntegration.isInitialized()) {
            Logger::get().logInfo("Initializing database for file history");
            if (!dbIntegration.initializeDatabase()) {
                Logger::get().logError("Failed to initialize database for file history");
                return {};
            }
        }

        // Check database connection
        auto& db = DatabaseManager::getInstance();
        if (!db.isConnected()) {
            Logger::get().logError("Database connection lost - cannot retrieve file history");
            return {};
        }

        auto files = dbIntegration.getAllFiles();
        Logger::get().logInfo("Retrieved " + std::to_string(files.size()) + " files from history");

        // Log file history for debugging
        if (!files.empty()) {
            return files;
        } else {
            Logger::get().logInfo("No files found in database - this is normal for first use");
            return {};
        }
    } catch (const std::exception& e) {
        Logger::get().logError("Exception during file history retrieval: " + string(e.what()));
        return {};
    }
}

bool Model::deleteFileFromHistory(int fileId) {
    try {
        auto& dbIntegration = ModelDatabaseIntegration::getInstance();
        if (!dbIntegration.isInitialized()) {
            Logger::get().logInfo("Initializing database for file deletion");
            if (!dbIntegration.initializeDatabase()) {
                Logger::get().logError("Failed to initialize database for file deletion");
                return false;
            }
        }

        auto& db = DatabaseManager::getInstance();
        if (!db.isConnected()) {
            Logger::get().logError("Database not connected for file deletion");
            return false;
        }

        // Get file details before deletion for logging
        FileEntity file = db.files()->getFileById(fileId);
        if (file.id != 0) {
            // Get count of courses
            int courseCount = db.courses()->getCourseCountByFileId(fileId);

            DatabaseTransaction transaction(db);

            // Delete courses first
            if (!db.courses()->deleteCoursesByFileId(fileId)) {
                Logger::get().logError("Failed to delete courses for file ID: " + std::to_string(fileId));
                return false;
            }

            // Delete the file record
            if (!db.files()->deleteFile(fileId)) {
                Logger::get().logError("Failed to delete file record for ID: " + std::to_string(fileId));
                return false;
            }

            // Commit the transaction
            if (!transaction.commit()) {
                Logger::get().logError("Failed to commit file deletion transaction");
                return false;
            }

            Logger::get().logInfo("Successfully deleted file '" + file.file_name + "' and " +
                                  std::to_string(courseCount) + " associated courses");
            return true;
        } else {
            Logger::get().logError("File with ID " + std::to_string(fileId) + " not found");
            return false;
        }

    } catch (const std::exception& e) {
        Logger::get().logError("Exception during file deletion: " + string(e.what()));
        return false;
    }
}

vector<string> Model::validateCourses(const vector<Course>& courses) {
    if (courses.empty()) {
        Logger::get().logError("No courses were found to validate");
        Logger::get().stopCollecting();
        return {}; // Return empty vector
    }

    Logger::get().logInfo("Validating " + std::to_string(courses.size()) + " courses");

    vector<string> validationErrors = validate_courses(courses);

    vector<string> allCollectedMessages = Logger::get().getAllCollectedMessages();

    for (const auto& error : validationErrors) {
        allCollectedMessages.push_back("[Validation] " + error);
    }

    Logger::get().stopCollecting();
    Logger::get().clearCollected();

    return allCollectedMessages;
}

vector<InformativeSchedule> Model::generateSchedules(const vector<Course>& userInput) {
    if (userInput.empty() || userInput.size() > 8) {
        Logger::get().logError("invalid amount of courses (" + std::to_string(userInput.size()) + "), aborting...");
        return {};
    }

    Logger::get().logInfo("Generating schedules for " + std::to_string(userInput.size()) + " courses");

    ScheduleBuilder builder;
    vector<InformativeSchedule> schedules = builder.build(userInput);

    if (schedules.empty()) {
        Logger::get().logError("unable to generate schedules, aborting process");
        return schedules;
    }

    Logger::get().logInfo("Generated " + std::to_string(schedules.size()) + " possible schedules");

    return schedules;
}

void Model::saveSchedule(const InformativeSchedule& infoSchedule, const string& path) {
    bool status = saveScheduleToCsv(path, infoSchedule);
    string message = status ? "Schedule saved to CSV: " + path : "An error has occurred, unable to save schedule as csv";
    Logger::get().logInfo(message);
}

void Model::printSchedule(const InformativeSchedule& infoSchedule) {
    bool status = printSelectedSchedule(infoSchedule);
    string message = status ? "Schedule sent to printer" : "An error has occurred, unable to print schedule";
    Logger::get().logInfo(message);
}

vector<string> Model::messageBot(const vector<string>& userInput, const string& data) {
    return askModel(userInput[0], data);
}

void* Model::executeOperation(ModelOperation operation, const void* data, const string& path) {
    switch (operation) {
        case ModelOperation::GENERATE_COURSES: {
            if (!path.empty()) {
                lastGeneratedCourses = generateCourses(path);
                return &lastGeneratedCourses;
            } else {
                Logger::get().logError("File not found, aborting...");
                return nullptr;
            }
        }

        case ModelOperation::LOAD_FROM_HISTORY: {
            if (data) {
                const auto* fileLoadData = static_cast<const FileLoadData*>(data);
                lastGeneratedCourses = loadCoursesFromHistory(fileLoadData->fileIds);
                return &lastGeneratedCourses;
            } else {
                Logger::get().logError("No file IDs provided for history loading");
                return nullptr;
            }
        }

        case ModelOperation::GET_FILE_HISTORY: {
            auto* fileHistory = new vector<FileEntity>(getFileHistory());
            return fileHistory;
        }

        case ModelOperation::DELETE_FILE_FROM_HISTORY: {
            if (data) {
                const int* fileId = static_cast<const int*>(data);
                bool success = deleteFileFromHistory(*fileId);
                bool* result = new bool(success);
                return result;
            } else {
                Logger::get().logError("No file ID provided for deletion");
                return nullptr;
            }
        }

        case ModelOperation::VALIDATE_COURSES: {
            if (data) {
                const auto* courses = static_cast<const vector<Course>*>(data);
                auto* validationResult = new vector<string>(validateCourses(*courses));
                return validationResult;
            } else {
                Logger::get().logError("No courses were found for validation, aborting...");
                return nullptr;
            }
        }

        case ModelOperation::GENERATE_SCHEDULES: {
            if (data) {
                const auto* courses = static_cast<const vector<Course>*>(data);
                lastGeneratedSchedules = generateSchedules(*courses);
                scheduleMetaData = CalculateMetaData(lastGeneratedSchedules);
                return &lastGeneratedSchedules;
            } else {
                Logger::get().logError("unable to generate schedules, aborting...");
                return nullptr;
            }
        }

        case ModelOperation::SAVE_SCHEDULE: {
            if (data && !path.empty()) {
                const auto* schedule = static_cast<const InformativeSchedule*>(data);
                saveSchedule(*schedule, path);
            } else {
                Logger::get().logError("unable to save schedule, aborting...");
            }
            break;
        }

        case ModelOperation::PRINT_SCHEDULE: {
            if (data) {
                const auto* schedule = static_cast<const InformativeSchedule*>(data);
                printSchedule(*schedule);
            } else {
                Logger::get().logError("unable to print schedule, aborting...");
            }
            break;
        }

        case ModelOperation::LOAD_COURSES_FROM_DB: {
            try {
                lastGeneratedCourses = loadCoursesFromDB();
                return &lastGeneratedCourses;
            } catch (const std::exception& e) {
                Logger::get().logError("Failed to load courses from database: " + string(e.what()));
                return nullptr;
            }
        }

        case ModelOperation::CLEAR_DATABASE: {
            try {
                Logger::get().logInfo("=== CLEARING DATABASE ===");
                auto& dbIntegration = ModelDatabaseIntegration::getInstance();
                if (dbIntegration.isInitialized()) {
                    bool success = dbIntegration.clearAllDatabaseData();
                    Logger::get().logInfo("Database clear result: " + string(success ? "SUCCESS" : "FAILED"));
                    bool* result = new bool(success);
                    return result;
                }
                Logger::get().logError("Database not initialized for clearing");
                return nullptr;
            } catch (const std::exception& e) {
                Logger::get().logError("Failed to clear database: " + string(e.what()));
                return nullptr;
            }
        }

        case ModelOperation::BOT_MESSAGE: {
            if (data) {
                const auto* userInput = static_cast<const vector<string>*>(data);
                auto* botRespond = new vector<string>(messageBot(*userInput, scheduleMetaData));
                return botRespond;
            } else {
                Logger::get().logError("invalid message");
                return nullptr;
            }
        }

        case ModelOperation::BACKUP_DATABASE: {
            Logger::get().logWarning("Database backup not yet implemented");
            break;
        }

        case ModelOperation::RESTORE_DATABASE: {
            Logger::get().logWarning("Database restore not yet implemented");
            break;
        }

        case ModelOperation::GET_DATABASE_STATS: {
            try {
                auto& dbIntegration = ModelDatabaseIntegration::getInstance();
                if (dbIntegration.isInitialized()) {
                    auto* stats = new ModelDatabaseIntegration::DatabaseStats(dbIntegration.getDatabaseStats());
                    return stats;
                }
                return nullptr;
            } catch (const std::exception& e) {
                Logger::get().logError("Failed to get database stats: " + string(e.what()));
                return nullptr;
            }
        }
    }
    return nullptr;
}