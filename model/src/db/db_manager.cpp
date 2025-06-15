#include "db_manager.h"
#include "db_json_helpers.h"

DatabaseManager& DatabaseManager::getInstance() {
    static DatabaseManager instance;
    return instance;
}

DatabaseManager::~DatabaseManager() {
    closeDatabase();
}

bool DatabaseManager::initializeDatabase(const QString& dbPath) {
    if (isInitialized && db.isOpen()) {
        return true;
    }

    closeDatabase();

    QString databasePath = dbPath;
    if (databasePath.isEmpty()) {
        // Use application data directory
        QString appDataPath = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
        QDir().mkpath(appDataPath);
        databasePath = QDir(appDataPath).filePath("schedulify.db");
    }

    Logger::get().logInfo("Initializing database at: " + databasePath.toStdString());

    // Setup SQLite connection
    db = QSqlDatabase::addDatabase("QSQLITE", "schedulify_connection");
    db.setDatabaseName(databasePath);

    if (!db.open()) {
        Logger::get().logError("Failed to open database: " + db.lastError().text().toStdString());
        return false;
    }

    // Enable foreign keys
    QSqlQuery pragmaQuery(db);
    if (!pragmaQuery.exec("PRAGMA foreign_keys = ON")) {
        Logger::get().logWarning("Failed to enable foreign keys: " + pragmaQuery.lastError().text().toStdString());
    }

    // Initialize managers
    schemaManager = std::make_unique<DatabaseSchema>(db);
    fileManager = std::make_unique<DatabaseFileManager>(db);
    courseManager = std::make_unique<DatabaseCourseManager>(db);

    // Create tables and indexes
    if (!schemaManager->createTables()) {
        Logger::get().logError("Failed to create database tables");
        closeDatabase();
        return false;
    }

    if (!schemaManager->createIndexes()) {
        Logger::get().logWarning("Failed to create some database indexes");
    }

    // Initialize schema version metadata
    if (getMetadata("schema_version").empty()) {
        insertMetadata("schema_version", std::to_string(getCurrentSchemaVersion()), "Database schema version");
        insertMetadata("created_at", QDateTime::currentDateTime().toString(Qt::ISODate).toStdString(), "Database creation timestamp");
    }

    // Validate schema
    if (!schemaManager->validateSchema()) {
        Logger::get().logError("Database schema validation failed");
        closeDatabase();
        return false;
    }

    isInitialized = true;
    Logger::get().logInfo("Database initialized successfully");
    return true;
}

bool DatabaseManager::isConnected() const {
    return isInitialized && db.isOpen();
}

void DatabaseManager::closeDatabase() {
    // Reset managers before closing database
    courseManager.reset();
    fileManager.reset();
    schemaManager.reset();

    if (db.isOpen()) {
        db.close();
    }
    QSqlDatabase::removeDatabase("schedulify_connection");
    isInitialized = false;
}

// Metadata operations
bool DatabaseManager::insertMetadata(const string& key, const string& value, const string& description) {
    if (!isConnected()) return false;

    QSqlQuery query(db);
    query.prepare(R"(
        INSERT OR REPLACE INTO metadata (key, value, description, updated_at)
        VALUES (?, ?, ?, CURRENT_TIMESTAMP)
    )");

    query.addBindValue(QString::fromStdString(key));
    query.addBindValue(QString::fromStdString(value));
    query.addBindValue(QString::fromStdString(description));

    if (!query.exec()) {
        Logger::get().logError("Failed to insert metadata: " + query.lastError().text().toStdString());
        return false;
    }

    return true;
}

bool DatabaseManager::updateMetadata(const string& key, const string& value) {
    return insertMetadata(key, value, ""); // INSERT OR REPLACE handles updates
}

string DatabaseManager::getMetadata(const string& key, const string& defaultValue) {
    if (!isConnected()) return defaultValue;

    QSqlQuery query(db);
    query.prepare("SELECT value FROM metadata WHERE key = ?");
    query.addBindValue(QString::fromStdString(key));

    if (query.exec() && query.next()) {
        return query.value(0).toString().toStdString();
    }

    return defaultValue;
}

vector<MetadataEntity> DatabaseManager::getAllMetadata() {
    vector<MetadataEntity> metadata;
    if (!isConnected()) return metadata;

    QSqlQuery query("SELECT id, key, value, description, updated_at FROM metadata ORDER BY key", db);

    while (query.next()) {
        MetadataEntity entity;
        entity.id = query.value(0).toInt();
        entity.key = query.value(1).toString().toStdString();
        entity.value = query.value(2).toString().toStdString();
        entity.description = query.value(3).toString().toStdString();
        entity.updated_at = query.value(4).toDateTime();
        metadata.push_back(entity);
    }

    return metadata;
}

// Schedule operations (TODO: move to separate manager)
bool DatabaseManager::insertSchedule(const InformativeSchedule& schedule, const vector<int>& courseIds) {
    if (!isConnected()) return false;

    // Create course IDs JSON
    QJsonArray courseArray;
    for (int id : courseIds) {
        courseArray.append(id);
    }
    QJsonDocument courseDoc(courseArray);
    string courseJson = courseDoc.toJson(QJsonDocument::Compact).toStdString();

    // Create week JSON
    string weekJson = scheduleToJson(schedule);

    QSqlQuery query(db);
    query.prepare(R"(
        INSERT INTO schedule
        (schedule_index, courses_json, week_json, amount_days, amount_gaps, gaps_time, avg_start, avg_end)
        VALUES (?, ?, ?, ?, ?, ?, ?, ?)
    )");

    query.addBindValue(schedule.index);
    query.addBindValue(QString::fromStdString(courseJson));
    query.addBindValue(QString::fromStdString(weekJson));
    query.addBindValue(schedule.amount_days);
    query.addBindValue(schedule.amount_gaps);
    query.addBindValue(schedule.gaps_time);
    query.addBindValue(schedule.avg_start);
    query.addBindValue(schedule.avg_end);

    if (!query.exec()) {
        Logger::get().logError("Failed to insert schedule: " + query.lastError().text().toStdString());
        return false;
    }

    return true;
}

bool DatabaseManager::insertSchedules(const vector<InformativeSchedule>& schedules,
                                      const vector<vector<int>>& allCourseIds) {
    if (!isConnected()) return false;
    if (schedules.size() != allCourseIds.size()) {
        Logger::get().logError("Schedules and course IDs vectors size mismatch");
        return false;
    }

    DatabaseTransaction transaction(*this);

    for (size_t i = 0; i < schedules.size(); ++i) {
        if (!insertSchedule(schedules[i], allCourseIds[i])) {
            return false;
        }
    }

    return transaction.commit();
}

bool DatabaseManager::deleteAllSchedules() {
    if (!isConnected()) return false;

    QSqlQuery query("DELETE FROM schedule", db);
    if (!query.exec()) {
        Logger::get().logError("Failed to delete all schedules: " + query.lastError().text().toStdString());
        return false;
    }

    Logger::get().logInfo("Deleted all schedules from database");
    return true;
}

vector<InformativeSchedule> DatabaseManager::getAllSchedules() {
    vector<InformativeSchedule> schedules;
    if (!isConnected()) return schedules;

    QSqlQuery query(R"(
        SELECT id, schedule_index, week_json, amount_days, amount_gaps, gaps_time, avg_start, avg_end
        FROM schedule ORDER BY schedule_index
    )", db);

    while (query.next()) {
        InformativeSchedule schedule = scheduleFromJson(
                query.value(2).toString().toStdString(),  // week_json
                query.value(0).toInt(),                   // id
                query.value(1).toInt(),                   // schedule_index
                query.value(3).toInt(),                   // amount_days
                query.value(4).toInt(),                   // amount_gaps
                query.value(5).toInt(),                   // gaps_time
                query.value(6).toInt(),                   // avg_start
                query.value(7).toInt()                    // avg_end
        );
        schedules.push_back(schedule);
    }

    Logger::get().logInfo("Loaded " + std::to_string(schedules.size()) + " schedules from database");
    return schedules;
}

InformativeSchedule DatabaseManager::getScheduleById(int id) {
    InformativeSchedule schedule;
    if (!isConnected()) return schedule;

    QSqlQuery query(db);
    query.prepare(R"(
        SELECT id, schedule_index, week_json, amount_days, amount_gaps, gaps_time, avg_start, avg_end
        FROM schedule WHERE id = ?
    )");
    query.addBindValue(id);

    if (query.exec() && query.next()) {
        schedule = scheduleFromJson(
                query.value(2).toString().toStdString(),  // week_json
                query.value(0).toInt(),                   // id
                query.value(1).toInt(),                   // schedule_index
                query.value(3).toInt(),                   // amount_days
                query.value(4).toInt(),                   // amount_gaps
                query.value(5).toInt(),                   // gaps_time
                query.value(6).toInt(),                   // avg_start
                query.value(7).toInt()                    // avg_end
        );
    }

    return schedule;
}

// Schedule metadata operations (TODO: move to separate manager)
bool DatabaseManager::insertScheduleMetadata(int totalSchedules, const string& generationSettings) {
    if (!isConnected()) return false;

    QSqlQuery query(db);
    query.prepare(R"(
        INSERT INTO schedule_metadata (total_schedules, generation_settings_json)
        VALUES (?, ?)
    )");

    query.addBindValue(totalSchedules);
    query.addBindValue(QString::fromStdString(generationSettings));

    if (!query.exec()) {
        Logger::get().logError("Failed to insert schedule metadata: " + query.lastError().text().toStdString());
        return false;
    }

    return true;
}

bool DatabaseManager::updateScheduleMetadata(int id, const string& status) {
    if (!isConnected()) return false;

    QSqlQuery query(db);
    query.prepare("UPDATE schedule_metadata SET status = ? WHERE id = ?");
    query.addBindValue(QString::fromStdString(status));
    query.addBindValue(id);

    if (!query.exec()) {
        Logger::get().logError("Failed to update schedule metadata: " + query.lastError().text().toStdString());
        return false;
    }

    return true;
}

vector<ScheduleMetadataEntity> DatabaseManager::getAllScheduleMetadata() {
    vector<ScheduleMetadataEntity> metadata;
    if (!isConnected()) return metadata;

    QSqlQuery query(R"(
        SELECT id, total_schedules, generation_settings_json, generated_at, status
        FROM schedule_metadata ORDER BY generated_at DESC
    )", db);

    while (query.next()) {
        ScheduleMetadataEntity entity;
        entity.id = query.value(0).toInt();
        entity.total_schedules = query.value(1).toInt();
        entity.generation_settings_json = query.value(2).toString().toStdString();
        entity.generated_at = query.value(3).toDateTime();
        entity.status = query.value(4).toString().toStdString();
        metadata.push_back(entity);
    }

    return metadata;
}

ScheduleMetadataEntity DatabaseManager::getLatestScheduleMetadata() {
    ScheduleMetadataEntity entity;
    if (!isConnected()) return entity;

    QSqlQuery query(R"(
        SELECT id, total_schedules, generation_settings_json, generated_at, status
        FROM schedule_metadata ORDER BY generated_at DESC LIMIT 1
    )", db);

    if (query.next()) {
        entity.id = query.value(0).toInt();
        entity.total_schedules = query.value(1).toInt();
        entity.generation_settings_json = query.value(2).toString().toStdString();
        entity.generated_at = query.value(3).toDateTime();
        entity.status = query.value(4).toString().toStdString();
    }

    return entity;
}

// Utility operations
bool DatabaseManager::clearAllData() {
    if (!isConnected()) return false;

    DatabaseTransaction transaction(*this);

    QStringList tables = {"schedule_metadata", "schedule", "course", "file", "metadata"};

    for (const QString& table : tables) {
        QSqlQuery query(db);
        if (!query.exec("DELETE FROM " + table)) {
            Logger::get().logError("Failed to clear table " + table.toStdString() + ": " +
                                   query.lastError().text().toStdString());
            return false;
        }
    }

    Logger::get().logInfo("Cleared all data from database");
    return transaction.commit();
}

bool DatabaseManager::exportToFile(const QString& filePath) {
    // TODO: Implement database export functionality
    Logger::get().logWarning("Database export functionality not yet implemented");
    return false;
}

bool DatabaseManager::importFromFile(const QString& filePath) {
    // TODO: Implement database import functionality
    Logger::get().logWarning("Database import functionality not yet implemented");
    return false;
}

int DatabaseManager::getTableRowCount(const string& tableName) {
    if (!isConnected()) return -1;

    QSqlQuery query(db);
    query.prepare("SELECT COUNT(*) FROM " + QString::fromStdString(tableName));

    if (query.exec() && query.next()) {
        return query.value(0).toInt();
    }

    return -1;
}

// Transaction support
bool DatabaseManager::beginTransaction() {
    if (!isConnected()) return false;
    return db.transaction();
}

bool DatabaseManager::commitTransaction() {
    if (!isConnected()) return false;
    return db.commit();
}

bool DatabaseManager::rollbackTransaction() {
    if (!isConnected()) return false;
    return db.rollback();
}

// Internal methods
bool DatabaseManager::executeQuery(const QString& query, const QVariantList& params) {
    QSqlQuery sqlQuery(db);
    sqlQuery.prepare(query);

    for (const QVariant& param : params) {
        sqlQuery.addBindValue(param);
    }

    if (!sqlQuery.exec()) {
        Logger::get().logError("Failed to execute query: " + sqlQuery.lastError().text().toStdString());
        return false;
    }

    return true;
}

QSqlQuery DatabaseManager::prepareQuery(const QString& query) {
    QSqlQuery sqlQuery(db);
    sqlQuery.prepare(query);
    return sqlQuery;
}

// Schedule JSON conversion helpers (TODO: move to separate helper class)
string DatabaseManager::scheduleToJson(const InformativeSchedule& schedule) {
    return DatabaseJsonHelpers::scheduleToJson(schedule);
}

InformativeSchedule DatabaseManager::scheduleFromJson(const string& json, int id, int schedule_index,
                                                      int amount_days, int amount_gaps, int gaps_time,
                                                      int avg_start, int avg_end) {
    return DatabaseJsonHelpers::scheduleFromJson(json, id, schedule_index, amount_days, amount_gaps, gaps_time, avg_start, avg_end);
}

// DatabaseTransaction implementation
DatabaseTransaction::DatabaseTransaction(DatabaseManager& dbManager) : db(dbManager) {
    db.beginTransaction();
}

DatabaseTransaction::~DatabaseTransaction() {
    if (!committed && !rolledBack) {
        rollback();
    }
}

bool DatabaseTransaction::commit() {
    if (committed || rolledBack) return false;

    committed = db.commitTransaction();
    return committed;
}

void DatabaseTransaction::rollback() {
    if (committed || rolledBack) return;

    db.rollbackTransaction();
    rolledBack = true;
}