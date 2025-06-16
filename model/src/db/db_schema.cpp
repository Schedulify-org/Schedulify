#include "db_schema.h"

DatabaseSchema::DatabaseSchema(QSqlDatabase& database) : db(database) {
}

bool DatabaseSchema::createTables() {
    return createMetadataTable() &&
           createFileTable() &&
           createCourseTable();
}

bool DatabaseSchema::createIndexes() {
    return createMetadataIndexes() &&
           createFileIndexes() &&
           createCourseIndexes();
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

bool DatabaseSchema::validateSchema() {
    QStringList requiredTables = {"metadata", "file", "course"};

    for (const QString& table : requiredTables) {
        if (!tableExists(table)) {
            Logger::get().logError("Required table missing: " + table.toStdString());
            return false;
        }
    }

    // Validate course table has required columns including course_file_id
    if (!validateCourseTableColumns()) {
        Logger::get().logError("Course table validation failed");
        return false;
    }

    // Validate file table has required columns
    if (!validateFileTableColumns()) {
        Logger::get().logError("File table validation failed");
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