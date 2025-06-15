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
        Logger::get().logInfo("=== STARTING FILE UPLOAD PROCESS ===");
        Logger::get().logInfo("File path: " + path);

        // CRITICAL: Initialize database integration FIRST with error checking
        auto& dbIntegration = ModelDatabaseIntegration::getInstance();
        if (!dbIntegration.isInitialized()) {
            Logger::get().logInfo("Initializing database for first time...");
            if (!dbIntegration.initializeDatabase()) {
                Logger::get().logError("CRITICAL: Failed to initialize database - proceeding without persistence");
                // Continue without database - don't fail the entire operation
            } else {
                Logger::get().logInfo("Database initialized successfully");
            }
        }

        // Determine file type and use appropriate parser
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

            // Log first few course IDs for debugging
            Logger::get().logInfo("=== COURSE UPLOAD DEBUG ===");
            for (size_t i = 0; i < std::min(courses.size(), size_t(5)); ++i) {
                Logger::get().logInfo("Course " + std::to_string(i) + ": ID=" + std::to_string(courses[i].id) +
                                      ", Raw ID=" + courses[i].raw_id + ", Name=" + courses[i].name);
            }

            // Extract file information from path
            size_t lastSlash = path.find_last_of("/\\");
            string fileName = (lastSlash != string::npos) ? path.substr(lastSlash + 1) : path;
            string fileType = extension;

            Logger::get().logInfo("About to save " + std::to_string(courses.size()) + " courses to database");
            Logger::get().logInfo("File: " + fileName + ", Type: " + fileType);

            // REQUIREMENT 1 & 2: Try to load courses into database (but don't fail if it doesn't work)
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

    Logger::get().logInfo("=== FILE UPLOAD PROCESS COMPLETED ===");
    Logger::get().logInfo("Result: " + std::to_string(courses.size()) + " courses loaded");

    // Keep collecting enabled for validation phase
    return courses;
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

        // REQUIREMENT 3 & 4: Get courses by file IDs with conflict resolution
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
        Logger::get().logInfo("=== RETRIEVING FILE HISTORY ===");

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
            Logger::get().logInfo("=== FILE HISTORY DETAILS ===");
            for (const auto& file : files) {
                Logger::get().logInfo("File ID " + std::to_string(file.id) + ": '" + file.file_name +
                                      "' (type: " + file.file_type + ", uploaded: " +
                                      file.upload_time.toString("yyyy-MM-dd hh:mm:ss").toStdString() + ")");
            }
        } else {
            Logger::get().logInfo("No files found in database - this is normal for first use");
        }

        return files;
    } catch (const std::exception& e) {
        Logger::get().logError("Exception during file history retrieval: " + string(e.what()));
        return {};
    }
}

vector<string> Model::validateCourses(const vector<Course>& courses) {
    if (courses.empty()) {
        Logger::get().logError("No courses were found to validate");
        Logger::get().stopCollecting();
        return {}; // Return empty vector, not reference to member
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

    // Load schedules into database
    try {
        auto& dbIntegration = ModelDatabaseIntegration::getInstance();
        if (dbIntegration.isInitialized()) {
            if (dbIntegration.loadSchedulesToDatabase(schedules, userInput)) {
                Logger::get().logInfo("Schedules successfully loaded into database");
            } else {
                Logger::get().logWarning("Failed to load schedules into database, continuing without persistence");
            }
        }
    } catch (const std::exception& e) {
        Logger::get().logWarning("Database error while loading schedules: " + string(e.what()));
    }

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
        case ModelOperation::GENERATE_COURSES:
            if (!path.empty()) {
                Logger::get().logInfo("=== STARTING FILE UPLOAD PROCESS ===");
                Logger::get().logInfo("File path: " + path);
                lastGeneratedCourses = generateCourses(path);
                Logger::get().logInfo("=== FILE UPLOAD PROCESS COMPLETED ===");
                Logger::get().logInfo("Result: " + std::to_string(lastGeneratedCourses.size()) + " courses loaded");
                return &lastGeneratedCourses;
            } else {
                Logger::get().logError("File not found, aborting...");
                return nullptr;
            }

        case ModelOperation::LOAD_FROM_HISTORY:
            if (data) {
                const auto* fileLoadData = static_cast<const FileLoadData*>(data);
                Logger::get().logInfo("=== STARTING HISTORY LOAD PROCESS ===");
                lastGeneratedCourses = loadCoursesFromHistory(fileLoadData->fileIds);
                Logger::get().logInfo("=== HISTORY LOAD PROCESS COMPLETED ===");
                Logger::get().logInfo("Result: " + std::to_string(lastGeneratedCourses.size()) + " courses loaded from history");
                return &lastGeneratedCourses;
            } else {
                Logger::get().logError("No file IDs provided for history loading");
                return nullptr;
            }

        case ModelOperation::GET_FILE_HISTORY: {
            Logger::get().logInfo("=== RETRIEVING FILE HISTORY ===");
            auto* fileHistory = new vector<FileEntity>(getFileHistory());
            Logger::get().logInfo("File history retrieved: " + std::to_string(fileHistory->size()) + " files");
            return fileHistory;
        }

        case ModelOperation::VALIDATE_COURSES:
            if (data) {
                const auto* courses = static_cast<const vector<Course>*>(data);
                Logger::get().logInfo("=== STARTING COURSE VALIDATION ===");
                auto* validationResult = new vector<string>(validateCourses(*courses));
                Logger::get().logInfo("Course validation completed");
                return validationResult;
            } else {
                Logger::get().logError("No courses were found for validation, aborting...");
                return nullptr;
            }

        case ModelOperation::GENERATE_SCHEDULES:
            if (data) {
                const auto* courses = static_cast<const vector<Course>*>(data);
                Logger::get().logInfo("=== STARTING SCHEDULE GENERATION ===");
                lastGeneratedSchedules = generateSchedules(*courses);
                Logger::get().logInfo("=== SCHEDULE GENERATION COMPLETED ===");
                Logger::get().logInfo("Result: " + std::to_string(lastGeneratedSchedules.size()) + " schedules generated");
                return &lastGeneratedSchedules;
            } else {
                Logger::get().logError("unable to generate schedules, aborting...");
                return nullptr;
            }

        case ModelOperation::SAVE_SCHEDULE:
            if (data && !path.empty()) {
                const auto* schedule = static_cast<const InformativeSchedule*>(data);
                saveSchedule(*schedule, path);
            } else {
                Logger::get().logError("unable to save schedule, aborting...");
            }
            break;

        case ModelOperation::PRINT_SCHEDULE:
            if (data) {
                const auto* schedule = static_cast<const InformativeSchedule*>(data);
                printSchedule(*schedule);
            } else {
                Logger::get().logError("unable to print schedule, aborting...");
            }
            break;

        case ModelOperation::LOAD_COURSES_FROM_DB: {
            try {
                Logger::get().logInfo("=== LOADING ALL COURSES FROM DATABASE ===");
                auto& dbIntegration = ModelDatabaseIntegration::getInstance();
                if (!dbIntegration.isInitialized()) {
                    dbIntegration.initializeDatabase();
                }
                lastGeneratedCourses = dbIntegration.getCoursesFromDatabase();
                Logger::get().logInfo("Loaded " + std::to_string(lastGeneratedCourses.size()) + " courses from database");
                return &lastGeneratedCourses;
            } catch (const std::exception& e) {
                Logger::get().logError("Failed to load courses from database: " + string(e.what()));
                return nullptr;
            }
        }

        case ModelOperation::LOAD_SCHEDULES_FROM_DB: {
            try {
                Logger::get().logInfo("=== LOADING SCHEDULES FROM DATABASE ===");
                auto& dbIntegration = ModelDatabaseIntegration::getInstance();
                if (!dbIntegration.isInitialized()) {
                    dbIntegration.initializeDatabase();
                }
                lastGeneratedSchedules = dbIntegration.getSchedulesFromDatabase();
                Logger::get().logInfo("Loaded " + std::to_string(lastGeneratedSchedules.size()) + " schedules from database");
                return &lastGeneratedSchedules;
            } catch (const std::exception& e) {
                Logger::get().logError("Failed to load schedules from database: " + string(e.what()));
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

        case ModelOperation::BOT_MESSAGE:
            if (data) {
                const auto* userInput = static_cast<const vector<string>*>(data);
                auto* botRespond = new vector<string>(messageBot(*userInput, scheduleMetaData));
                return botRespond;
            } else {
                Logger::get().logError("invalid message");
                return nullptr;
            }

        case ModelOperation::BACKUP_DATABASE:
            Logger::get().logWarning("Database backup not yet implemented");
            break;

        case ModelOperation::RESTORE_DATABASE:
            Logger::get().logWarning("Database restore not yet implemented");
            break;

        case ModelOperation::GET_DATABASE_STATS:
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
    return nullptr;
}