#include "db_schema.h"
#include "logger.h"
#include <QSqlQuery>
#include <QSqlError>

DatabaseSchema::DatabaseSchema(QSqlDatabase& database) : db(database) {
}

bool DatabaseSchema::createTables() {
    Logger::get().logInfo("Creating database tables...");

    return createMetadataTable() &&
           createFileTable() &&
           createCourseTable() &&
           createScheduleTable() &&
           createScheduleMetadataTable();
}

bool DatabaseSchema::createIndexes() {
    Logger::get().logInfo("Creating database indexes...");

    return createMetadataIndexes() &&
           createFileIndexes() &&
           createCourseIndexes() &&
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

    return executeQuery(query);
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

    return executeQuery(query);
}

bool DatabaseSchema::createCourseTable() {
    const QString query = R"(
        CREATE TABLE IF NOT EXISTS course (
            id INTEGER PRIMARY KEY,
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
            UNIQUE(raw_id, file_id)
        )
    )";

    return executeQuery(query);
}

bool DatabaseSchema::createScheduleTable() {
    const QString query = R"(
        CREATE TABLE IF NOT EXISTS schedule (
            id INTEGER PRIMARY KEY AUTOINCREMENT,
            schedule_index INTEGER NOT NULL,
            courses_json TEXT NOT NULL,
            week_json TEXT NOT NULL,
            amount_days INTEGER DEFAULT 0,
            amount_gaps INTEGER DEFAULT 0,
            gaps_time INTEGER DEFAULT 0,
            avg_start INTEGER DEFAULT 0,
            avg_end INTEGER DEFAULT 0,
            created_at DATETIME DEFAULT CURRENT_TIMESTAMP
        )
    )";

    return executeQuery(query);
}

bool DatabaseSchema::createScheduleMetadataTable() {
    const QString query = R"(
        CREATE TABLE IF NOT EXISTS schedule_metadata (
            id INTEGER PRIMARY KEY AUTOINCREMENT,
            total_schedules INTEGER NOT NULL,
            generation_settings_json TEXT DEFAULT '{}',
            generated_at DATETIME DEFAULT CURRENT_TIMESTAMP,
            status TEXT DEFAULT 'active'
        )
    )";

    return executeQuery(query);
}

bool DatabaseSchema::createMetadataIndexes() {
    return executeQuery("CREATE INDEX IF NOT EXISTS idx_metadata_key ON metadata(key)");
}

bool DatabaseSchema::createFileIndexes() {
    return executeQuery("CREATE INDEX IF NOT EXISTS idx_file_name ON file(file_name)") &&
           executeQuery("CREATE INDEX IF NOT EXISTS idx_file_type ON file(file_type)") &&
           executeQuery("CREATE INDEX IF NOT EXISTS idx_file_upload_time ON file(upload_time)");
}

bool DatabaseSchema::createCourseIndexes() {
    return executeQuery("CREATE INDEX IF NOT EXISTS idx_course_raw_id ON course(raw_id)") &&
           executeQuery("CREATE INDEX IF NOT EXISTS idx_course_name ON course(name)") &&
           executeQuery("CREATE INDEX IF NOT EXISTS idx_course_file_id ON course(file_id)") &&
           executeQuery("CREATE INDEX IF NOT EXISTS idx_course_raw_id_file_id ON course(raw_id, file_id)");
}

bool DatabaseSchema::createScheduleIndexes() {
    return executeQuery("CREATE INDEX IF NOT EXISTS idx_schedule_index ON schedule(schedule_index)");
}

bool DatabaseSchema::dropAllTables() {
    Logger::get().logInfo("Dropping all database tables...");

    QStringList tables = {"schedule_metadata", "schedule", "course", "file", "metadata"};

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
    Logger::get().logInfo("Upgrading schema from version " + std::to_string(fromVersion) +
                          " to " + std::to_string(toVersion));

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
    Logger::get().logInfo("Upgrading from schema v1 to v2...");

    // Example upgrade: Add upload_time column to file table if it doesn't exist
    // Check if column exists first
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
        if (!executeQuery("ALTER TABLE file ADD COLUMN upload_time DATETIME DEFAULT CURRENT_TIMESTAMP")) {
            return false;
        }
        Logger::get().logInfo("Added upload_time column to file table");
    }

    // Add unique constraint to course table
    if (!executeQuery("CREATE UNIQUE INDEX IF NOT EXISTS idx_course_unique_raw_id_file ON course(raw_id, file_id)")) {
        return false;
    }

    Logger::get().logInfo("Schema upgrade v1->v2 completed successfully");
    return true;
}

bool DatabaseSchema::validateSchema() {
    QStringList requiredTables = {"metadata", "file", "course", "schedule", "schedule_metadata"};

    for (const QString& table : requiredTables) {
        if (!tableExists(table)) {
            Logger::get().logError("Required table missing: " + table.toStdString());
            return false;
        }
    }

    Logger::get().logInfo("Schema validation passed");
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
        Logger::get().logError("Query was: " + query.toStdString());
        return false;
    }
    return true;
}