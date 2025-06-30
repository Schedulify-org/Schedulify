#include "db_schema.h"

DatabaseSchema::DatabaseSchema(QSqlDatabase& database) : db(database) {
}

bool DatabaseSchema::createTables() {
    return createMetadataTable() &&
           createFileTable() &&
           createCourseTable() &&
           createScheduleSetTable() &&
           createScheduleTable();
}

bool DatabaseSchema::createIndexes() {
    return createMetadataIndexes() &&
           createFileIndexes() &&
           createCourseIndexes() &&
           createScheduleSetIndexes() &&
           createScheduleIndexes();
}

bool DatabaseSchema::createMetadataTable() {
    const QString query = R"(
        CREATE TABLE IF NOT EXISTS metadata (
            id INTEGER PRIMARY KEY AUTOINCREMENT,
            key TEXT UNIQUE NOT NULL,
            value TEXT NOT NULL,
            description TEXT,
            updated_at DATETIME DEFAULT CURRENT_TIMESTAMP
        )
    )";

    if (!executeQuery(query)) {
        Logger::get().logError("Failed to create metadata table");
        return false;
    }

    Logger::get().logInfo("Metadata table created successfully");
    return true;
}

bool DatabaseSchema::createFileTable() {
    const QString query = R"(
        CREATE TABLE IF NOT EXISTS file (
            id INTEGER PRIMARY KEY AUTOINCREMENT,
            file_name TEXT NOT NULL,
            file_type TEXT NOT NULL,
            upload_time DATETIME DEFAULT CURRENT_TIMESTAMP,
            updated_at DATETIME DEFAULT CURRENT_TIMESTAMP
        )
    )";

    if (!executeQuery(query)) {
        Logger::get().logError("Failed to create file table");
        return false;
    }

    Logger::get().logInfo("File table created successfully");
    return true;
}

bool DatabaseSchema::createCourseTable() {
    const QString query = R"(
        CREATE TABLE IF NOT EXISTS course (
            id INTEGER PRIMARY KEY AUTOINCREMENT,
            course_file_id INTEGER NOT NULL,
            raw_id TEXT NOT NULL,
            name TEXT NOT NULL,
            teacher TEXT NOT NULL,
            lectures_json TEXT DEFAULT '[]',
            tutorials_json TEXT DEFAULT '[]',
            labs_json TEXT DEFAULT '[]',
            blocks_json TEXT DEFAULT '[]',
            file_id INTEGER NOT NULL,
            created_at DATETIME DEFAULT CURRENT_TIMESTAMP,
            updated_at DATETIME DEFAULT CURRENT_TIMESTAMP,
            FOREIGN KEY (file_id) REFERENCES file(id) ON DELETE CASCADE,
            UNIQUE(course_file_id, file_id)
        )
    )";

    if (!executeQuery(query)) {
        Logger::get().logError("Failed to create course table");
        return false;
    }

    Logger::get().logInfo("Course table created successfully");
    return true;
}

bool DatabaseSchema::createMetadataIndexes() {
    Logger::get().logInfo("Creating metadata indexes...");

    if (!executeQuery("CREATE INDEX IF NOT EXISTS idx_metadata_key ON metadata(key)")) {
        Logger::get().logWarning("Failed to create metadata key index");
        return false;
    }

    Logger::get().logInfo("Metadata indexes created successfully");
    return true;
}

bool DatabaseSchema::createFileIndexes() {
    bool success = true;

    if (!executeQuery("CREATE INDEX IF NOT EXISTS idx_file_name ON file(file_name)")) {
        Logger::get().logWarning("Failed to create file name index");
        success = false;
    }

    if (!executeQuery("CREATE INDEX IF NOT EXISTS idx_file_type ON file(file_type)")) {
        Logger::get().logWarning("Failed to create file type index");
        success = false;
    }

    if (!executeQuery("CREATE INDEX IF NOT EXISTS idx_file_upload_time ON file(upload_time)")) {
        Logger::get().logWarning("Failed to create file upload_time index");
        success = false;
    }

    if (success) {
        Logger::get().logInfo("File indexes created successfully");
    }
    return success;
}

bool DatabaseSchema::createCourseIndexes() {
    bool success = true;

    if (!executeQuery("CREATE INDEX IF NOT EXISTS idx_course_raw_id ON course(raw_id)")) {
        Logger::get().logWarning("Failed to create course raw_id index");
        success = false;
    }

    if (!executeQuery("CREATE INDEX IF NOT EXISTS idx_course_name ON course(name)")) {
        Logger::get().logWarning("Failed to create course name index");
        success = false;
    }

    if (!executeQuery("CREATE INDEX IF NOT EXISTS idx_course_file_id ON course(file_id)")) {
        Logger::get().logWarning("Failed to create course file_id index");
        success = false;
    }

    if (!executeQuery("CREATE INDEX IF NOT EXISTS idx_course_course_file_id ON course(course_file_id)")) {
        Logger::get().logWarning("Failed to create course course_file_id index");
        success = false;
    }

    if (!executeQuery("CREATE INDEX IF NOT EXISTS idx_course_composite ON course(course_file_id, file_id)")) {
        Logger::get().logWarning("Failed to create course composite index");
        success = false;
    }

    if (success) {
        Logger::get().logInfo("Course indexes created successfully");
    }
    return success;
}

bool DatabaseSchema::dropAllTables() {
    QStringList tables = {"course", "file", "metadata"};

    for (const QString& table : tables) {
        if (!executeQuery("DROP TABLE IF EXISTS " + table)) {
            Logger::get().logError("Failed to drop table: " + table.toStdString());
            return false;
        }
    }

    Logger::get().logInfo("All tables dropped successfully");
    return true;
}

bool DatabaseSchema::upgradeSchema(int fromVersion, int toVersion) {
    if (fromVersion == toVersion) {
        return true; // No upgrade needed
    }

    if (fromVersion == 1 && toVersion == 2) {
        return upgradeFromV1ToV2();
    }

    // Add new upgrade path for schedule tables
    if (fromVersion == 2 && toVersion == 3) {
        return upgradeFromV2ToV3();
    }

    Logger::get().logError("Unsupported schema upgrade path: " + std::to_string(fromVersion) +
                           " -> " + std::to_string(toVersion));
    return false;
}

bool DatabaseSchema::upgradeFromV1ToV2() {
    // Check if upload_time column exists in file table
    QSqlQuery checkQuery("PRAGMA table_info(file)", db);
    bool hasUploadTime = false;

    while (checkQuery.next()) {
        QString columnName = checkQuery.value(1).toString();
        if (columnName == "upload_time") {
            hasUploadTime = true;
            break;
        }
    }

    if (!hasUploadTime) {
        Logger::get().logInfo("Adding upload_time column to file table...");
        if (!executeQuery("ALTER TABLE file ADD COLUMN upload_time DATETIME DEFAULT CURRENT_TIMESTAMP")) {
            Logger::get().logError("Failed to add upload_time column");
            return false;
        }
        Logger::get().logInfo("Added upload_time column to file table");
    }

    // Update existing records to have upload_time = updated_at if upload_time is NULL
    if (!executeQuery("UPDATE file SET upload_time = updated_at WHERE upload_time IS NULL")) {
        Logger::get().logWarning("Failed to update existing upload_time values");
    }

    // Check if course_file_id column exists
    QSqlQuery checkCourseQuery("PRAGMA table_info(course)", db);
    bool hasCourseFileId = false;

    while (checkCourseQuery.next()) {
        QString columnName = checkCourseQuery.value(1).toString();
        if (columnName == "course_file_id") {
            hasCourseFileId = true;
            break;
        }
    }

    if (!hasCourseFileId) {
        Logger::get().logInfo("Adding course_file_id column to course table...");

        // Add the new column
        if (!executeQuery("ALTER TABLE course ADD COLUMN course_file_id INTEGER")) {
            Logger::get().logError("Failed to add course_file_id column");
            return false;
        }

        // Copy existing id values to course_file_id
        if (!executeQuery("UPDATE course SET course_file_id = id WHERE course_file_id IS NULL")) {
            Logger::get().logError("Failed to migrate course IDs");
            return false;
        }

        Logger::get().logInfo("Added course_file_id column and migrated existing data");
    }

    Logger::get().logInfo("Schema upgrade v1->v2 completed successfully");
    return true;
}

bool DatabaseSchema::upgradeFromV2ToV3() {
    Logger::get().logInfo("Upgrading schema from v2 to v3 (adding enhanced schedule columns)");

    // First create schedule tables if they don't exist
    if (!createScheduleSetTable() || !createScheduleTable()) {
        Logger::get().logError("Failed to create schedule tables during upgrade");
        return false;
    }

    // Check if we need to add enhanced columns to existing schedule table
    QSqlQuery checkColumn("PRAGMA table_info(schedule)", db);
    bool hasEnhancedColumns = false;

    while (checkColumn.next()) {
        QString columnName = checkColumn.value(1).toString();
        if (columnName == "earliest_start") {
            hasEnhancedColumns = true;
            break;
        }
    }

    if (!hasEnhancedColumns) {
        Logger::get().logInfo("Adding enhanced columns to existing schedule table");

        // Add all the enhanced columns
        vector<QString> enhancedColumns = {
                "ALTER TABLE schedule ADD COLUMN earliest_start INTEGER DEFAULT 0",
                "ALTER TABLE schedule ADD COLUMN latest_end INTEGER DEFAULT 0",
                "ALTER TABLE schedule ADD COLUMN longest_gap INTEGER DEFAULT 0",
                "ALTER TABLE schedule ADD COLUMN total_class_time INTEGER DEFAULT 0",
                "ALTER TABLE schedule ADD COLUMN consecutive_days INTEGER DEFAULT 0",
                "ALTER TABLE schedule ADD COLUMN days_json TEXT DEFAULT '[]'",
                "ALTER TABLE schedule ADD COLUMN weekend_classes BOOLEAN DEFAULT 0",
                "ALTER TABLE schedule ADD COLUMN has_morning_classes BOOLEAN DEFAULT 0",
                "ALTER TABLE schedule ADD COLUMN has_early_morning BOOLEAN DEFAULT 0",
                "ALTER TABLE schedule ADD COLUMN has_evening_classes BOOLEAN DEFAULT 0",
                "ALTER TABLE schedule ADD COLUMN has_late_evening BOOLEAN DEFAULT 0",
                "ALTER TABLE schedule ADD COLUMN max_daily_hours INTEGER DEFAULT 0",
                "ALTER TABLE schedule ADD COLUMN min_daily_hours INTEGER DEFAULT 0",
                "ALTER TABLE schedule ADD COLUMN avg_daily_hours INTEGER DEFAULT 0",
                "ALTER TABLE schedule ADD COLUMN has_lunch_break BOOLEAN DEFAULT 0",
                "ALTER TABLE schedule ADD COLUMN max_daily_gaps INTEGER DEFAULT 0",
                "ALTER TABLE schedule ADD COLUMN avg_gap_length INTEGER DEFAULT 0",
                "ALTER TABLE schedule ADD COLUMN schedule_span INTEGER DEFAULT 0",
                "ALTER TABLE schedule ADD COLUMN compactness_ratio REAL DEFAULT 0.0",
                "ALTER TABLE schedule ADD COLUMN weekday_only BOOLEAN DEFAULT 0",
                "ALTER TABLE schedule ADD COLUMN has_monday BOOLEAN DEFAULT 0",
                "ALTER TABLE schedule ADD COLUMN has_tuesday BOOLEAN DEFAULT 0",
                "ALTER TABLE schedule ADD COLUMN has_wednesday BOOLEAN DEFAULT 0",
                "ALTER TABLE schedule ADD COLUMN has_thursday BOOLEAN DEFAULT 0",
                "ALTER TABLE schedule ADD COLUMN has_friday BOOLEAN DEFAULT 0",
                "ALTER TABLE schedule ADD COLUMN has_saturday BOOLEAN DEFAULT 0",
                "ALTER TABLE schedule ADD COLUMN has_sunday BOOLEAN DEFAULT 0"
        };

        for (const QString& alterQuery : enhancedColumns) {
            if (!executeQuery(alterQuery)) {
                Logger::get().logError("Failed to add column: " + alterQuery.toStdString());
                // Continue with other columns rather than failing completely
            }
        }
    }

    // Create indexes for enhanced columns
    if (!createScheduleIndexes()) {
        Logger::get().logWarning("Some enhanced indexes failed to create");
    }

    Logger::get().logInfo("Schema upgrade v2->v3 completed successfully");
    return true;
}

bool DatabaseSchema::validateSchema() {
    QStringList requiredTables = {"metadata", "file", "course", "schedule_set", "schedule"};

    for (const QString& table : requiredTables) {
        if (!tableExists(table)) {
            Logger::get().logError("Required table missing: " + table.toStdString());
            return false;
        }
    }

    // Validate all table columns
    if (!validateCourseTableColumns() ||
        !validateFileTableColumns() ||
        !validateScheduleSetTableColumns() ||
        !validateScheduleTableColumns()) {
        return false;
    }

    Logger::get().logInfo("Schema validation passed");
    return true;
}

bool DatabaseSchema::validateCourseTableColumns() {
    QSqlQuery query("PRAGMA table_info(course)", db);
    QStringList foundColumns;

    while (query.next()) {
        QString columnName = query.value(1).toString();
        foundColumns << columnName;
    }

    QStringList requiredColumns = {"id", "course_file_id", "raw_id", "name", "teacher",
                                   "lectures_json", "tutorials_json", "labs_json",
                                   "blocks_json", "file_id", "created_at", "updated_at"};

    for (const QString& required : requiredColumns) {
        if (!foundColumns.contains(required)) {
            Logger::get().logError("Course table missing required column: " + required.toStdString());
            return false;
        }
    }

    Logger::get().logInfo("Course table column validation passed");
    return true;
}

bool DatabaseSchema::validateFileTableColumns() {
    QSqlQuery query("PRAGMA table_info(file)", db);
    QStringList foundColumns;

    while (query.next()) {
        QString columnName = query.value(1).toString();
        foundColumns << columnName;
    }

    QStringList requiredColumns = {"id", "file_name", "file_type", "upload_time", "updated_at"};

    for (const QString& required : requiredColumns) {
        if (!foundColumns.contains(required)) {
            Logger::get().logError("File table missing required column: " + required.toStdString());
            return false;
        }
    }

    Logger::get().logInfo("File table column validation passed");
    return true;
}

bool DatabaseSchema::tableExists(const QString& tableName) {
    QSqlQuery query("SELECT name FROM sqlite_master WHERE type='table' AND name=?", db);
    query.addBindValue(tableName);

    if (query.exec() && query.next()) {
        return true;
    }

    return false;
}

bool DatabaseSchema::executeQuery(const QString& query) {
    QSqlQuery sqlQuery(db);
    if (!sqlQuery.exec(query)) {
        Logger::get().logError("Failed to execute query: " + sqlQuery.lastError().text().toStdString());
        return false;
    }
    return true;
}

bool DatabaseSchema::createScheduleSetTable() {
    const QString query = R"(
        CREATE TABLE IF NOT EXISTS schedule_set (
            id INTEGER PRIMARY KEY AUTOINCREMENT,
            set_name TEXT NOT NULL,
            source_file_ids_json TEXT DEFAULT '[]',
            schedule_count INTEGER DEFAULT 0,
            created_at DATETIME DEFAULT CURRENT_TIMESTAMP,
            updated_at DATETIME DEFAULT CURRENT_TIMESTAMP
        )
    )";

    if (!executeQuery(query)) {
        Logger::get().logError("Failed to create schedule_set table");
        return false;
    }

    Logger::get().logInfo("Schedule set table created successfully");
    return true;
}

bool DatabaseSchema::createScheduleTable() {
    const QString query = R"(
        CREATE TABLE IF NOT EXISTS schedule (
            id INTEGER PRIMARY KEY AUTOINCREMENT,
            schedule_set_id INTEGER NOT NULL,
            schedule_index INTEGER NOT NULL,
            schedule_name TEXT DEFAULT '',
            schedule_data_json TEXT NOT NULL,

            amount_days INTEGER DEFAULT 0,
            amount_gaps INTEGER DEFAULT 0,
            gaps_time INTEGER DEFAULT 0,
            avg_start INTEGER DEFAULT 0,
            avg_end INTEGER DEFAULT 0,

            earliest_start INTEGER DEFAULT 0,
            latest_end INTEGER DEFAULT 0,
            longest_gap INTEGER DEFAULT 0,
            total_class_time INTEGER DEFAULT 0,

            consecutive_days INTEGER DEFAULT 0,
            days_json TEXT DEFAULT '[]',
            weekend_classes BOOLEAN DEFAULT 0,

            has_morning_classes BOOLEAN DEFAULT 0,
            has_early_morning BOOLEAN DEFAULT 0,
            has_evening_classes BOOLEAN DEFAULT 0,
            has_late_evening BOOLEAN DEFAULT 0,

            max_daily_hours INTEGER DEFAULT 0,
            min_daily_hours INTEGER DEFAULT 0,
            avg_daily_hours INTEGER DEFAULT 0,

            has_lunch_break BOOLEAN DEFAULT 0,
            max_daily_gaps INTEGER DEFAULT 0,
            avg_gap_length INTEGER DEFAULT 0,

            schedule_span INTEGER DEFAULT 0,
            compactness_ratio REAL DEFAULT 0.0,

            weekday_only BOOLEAN DEFAULT 0,
            has_monday BOOLEAN DEFAULT 0,
            has_tuesday BOOLEAN DEFAULT 0,
            has_wednesday BOOLEAN DEFAULT 0,
            has_thursday BOOLEAN DEFAULT 0,
            has_friday BOOLEAN DEFAULT 0,
            has_saturday BOOLEAN DEFAULT 0,
            has_sunday BOOLEAN DEFAULT 0,

            created_at DATETIME DEFAULT CURRENT_TIMESTAMP,
            updated_at DATETIME DEFAULT CURRENT_TIMESTAMP,
            FOREIGN KEY (schedule_set_id) REFERENCES schedule_set(id) ON DELETE CASCADE,
            UNIQUE(schedule_set_id, schedule_index)
        )
    )";

    if (!executeQuery(query)) {
        Logger::get().logError("Failed to create schedule table");
        return false;
    }

    Logger::get().logInfo("Enhanced schedule table created successfully");
    return true;
}

bool DatabaseSchema::createScheduleSetIndexes() {
    bool success = true;

    if (!executeQuery("CREATE INDEX IF NOT EXISTS idx_schedule_set_name ON schedule_set(set_name)")) {
        Logger::get().logWarning("Failed to create schedule_set name index");
        success = false;
    }

    if (!executeQuery("CREATE INDEX IF NOT EXISTS idx_schedule_set_created_at ON schedule_set(created_at)")) {
        Logger::get().logWarning("Failed to create schedule_set created_at index");
        success = false;
    }

    if (success) {
        Logger::get().logInfo("Schedule set indexes created successfully");
    }
    return success;
}

bool DatabaseSchema::createScheduleIndexes() {
    bool success = true;

    // Existing indexes
    if (!executeQuery("CREATE INDEX IF NOT EXISTS idx_schedule_set_id ON schedule(schedule_set_id)")) {
        Logger::get().logWarning("Failed to create schedule set_id index");
        success = false;
    }

    if (!executeQuery("CREATE INDEX IF NOT EXISTS idx_schedule_index ON schedule(schedule_index)")) {
        Logger::get().logWarning("Failed to create schedule index index");
        success = false;
    }

    if (!executeQuery("CREATE INDEX IF NOT EXISTS idx_schedule_created_at ON schedule(created_at)")) {
        Logger::get().logWarning("Failed to create schedule created_at index");
        success = false;
    }

    // NEW: Comprehensive indexes for common query patterns

    // Time-based queries
    if (!executeQuery("CREATE INDEX IF NOT EXISTS idx_schedule_time_range ON schedule(earliest_start, latest_end)")) {
        Logger::get().logWarning("Failed to create schedule time range index");
        success = false;
    }

    // Morning/evening preferences
    if (!executeQuery("CREATE INDEX IF NOT EXISTS idx_schedule_time_preferences ON schedule(has_morning_classes, has_early_morning, has_evening_classes, has_late_evening)")) {
        Logger::get().logWarning("Failed to create schedule time preferences index");
        success = false;
    }

    // Basic metrics (most common queries)
    if (!executeQuery("CREATE INDEX IF NOT EXISTS idx_schedule_basic_metrics ON schedule(amount_days, amount_gaps, gaps_time)")) {
        Logger::get().logWarning("Failed to create schedule basic metrics index");
        success = false;
    }

    // Intensity and compactness
    if (!executeQuery("CREATE INDEX IF NOT EXISTS idx_schedule_intensity ON schedule(max_daily_hours, total_class_time, compactness_ratio)")) {
        Logger::get().logWarning("Failed to create schedule intensity index");
        success = false;
    }

    // Day patterns
    if (!executeQuery("CREATE INDEX IF NOT EXISTS idx_schedule_day_patterns ON schedule(consecutive_days, weekday_only, weekend_classes)")) {
        Logger::get().logWarning("Failed to create schedule day patterns index");
        success = false;
    }

    // Individual weekdays for specific day queries
    if (!executeQuery("CREATE INDEX IF NOT EXISTS idx_schedule_weekdays ON schedule(has_monday, has_tuesday, has_wednesday, has_thursday, has_friday)")) {
        Logger::get().logWarning("Failed to create schedule weekdays index");
        success = false;
    }

    // Gap patterns
    if (!executeQuery("CREATE INDEX IF NOT EXISTS idx_schedule_gaps ON schedule(longest_gap, avg_gap_length, has_lunch_break, max_daily_gaps)")) {
        Logger::get().logWarning("Failed to create schedule gaps index");
        success = false;
    }

    // Composite index for the most common "ideal schedule" queries
    if (!executeQuery("CREATE INDEX IF NOT EXISTS idx_schedule_ideal_combo ON schedule(amount_days, amount_gaps, has_morning_classes, has_evening_classes, weekday_only)")) {
        Logger::get().logWarning("Failed to create schedule ideal combo index");
        success = false;
    }

    if (success) {
        Logger::get().logInfo("Enhanced schedule indexes created successfully");
    }
    return success;
}

bool DatabaseSchema::validateScheduleSetTableColumns() {
    QSqlQuery query("PRAGMA table_info(schedule_set)", db);
    QStringList foundColumns;

    while (query.next()) {
        QString columnName = query.value(1).toString();
        foundColumns << columnName;
    }

    QStringList requiredColumns = {"id", "set_name", "source_file_ids_json", "schedule_count",
                                   "created_at", "updated_at"};

    for (const QString& required : requiredColumns) {
        if (!foundColumns.contains(required)) {
            Logger::get().logError("Schedule set table missing required column: " + required.toStdString());
            return false;
        }
    }

    Logger::get().logInfo("Schedule set table column validation passed");
    return true;
}

bool DatabaseSchema::validateScheduleTableColumns() {
    QSqlQuery query("PRAGMA table_info(schedule)", db);
    QStringList foundColumns;

    while (query.next()) {
        QString columnName = query.value(1).toString();
        foundColumns << columnName;
    }

    QStringList requiredColumns = {"id", "schedule_set_id", "schedule_index", "schedule_name",
                                   "schedule_data_json", "amount_days", "amount_gaps", "gaps_time",
                                   "avg_start", "avg_end", "created_at", "updated_at"};

    for (const QString& required : requiredColumns) {
        if (!foundColumns.contains(required)) {
            Logger::get().logError("Schedule table missing required column: " + required.toStdString());
            return false;
        }
    }

    Logger::get().logInfo("Schedule table column validation passed");
    return true;
}