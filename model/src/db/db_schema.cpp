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
    Logger::get().logInfo("Upgrading schema from v2 to v3 (adding schedule tables)");

    // Create schedule tables if they don't exist
    if (!createScheduleSetTable() || !createScheduleTable()) {
        Logger::get().logError("Failed to create schedule tables during upgrade");
        return false;
    }

    // Create indexes for schedule tables
    if (!createScheduleSetIndexes() || !createScheduleIndexes()) {
        Logger::get().logWarning("Some schedule indexes failed to create during upgrade");
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

    Logger::get().logInfo("Schedule table created successfully");
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

    if (!executeQuery("CREATE INDEX IF NOT EXISTS idx_schedule_set_id ON schedule(schedule_set_id)")) {
        Logger::get().logWarning("Failed to create schedule set_id index");
        success = false;
    }

    if (!executeQuery("CREATE INDEX IF NOT EXISTS idx_schedule_index ON schedule(schedule_index)")) {
        Logger::get().logWarning("Failed to create schedule index index");
        success = false;
    }

    if (!executeQuery("CREATE INDEX IF NOT EXISTS idx_schedule_metrics ON schedule(amount_days, amount_gaps, gaps_time)")) {
        Logger::get().logWarning("Failed to create schedule metrics index");
        success = false;
    }

    if (!executeQuery("CREATE INDEX IF NOT EXISTS idx_schedule_avg_times ON schedule(avg_start, avg_end)")) {
        Logger::get().logWarning("Failed to create schedule avg_times index");
        success = false;
    }

    if (!executeQuery("CREATE INDEX IF NOT EXISTS idx_schedule_created_at ON schedule(created_at)")) {
        Logger::get().logWarning("Failed to create schedule created_at index");
        success = false;
    }

    if (success) {
        Logger::get().logInfo("Schedule indexes created successfully");
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