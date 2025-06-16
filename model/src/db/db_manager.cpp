#include "db_manager.h"
#include "db_json_helpers.h"

class DatabaseRepair;

DatabaseManager& DatabaseManager::getInstance() {
    static DatabaseManager instance;
    return instance;
}

DatabaseManager::~DatabaseManager() {
    closeDatabase();
}

class DatabaseRepair {
public:
    static bool repairDatabase(DatabaseManager& dbManager) {
        if (!dbManager.isConnected()) {
            Logger::get().logError("Database not connected for repair");
            return false;
        }

        // Backup existing data if possible
        auto backupData = backupExistingData(dbManager);

        // Drop and recreate problematic tables
        if (!recreateSchema(dbManager)) {
            Logger::get().logError("Failed to recreate schema");
            return false;
        }

        // Restore data if we had any
        if (!restoreData(dbManager, backupData)) {
            Logger::get().logWarning("Some data could not be restored");
        }

        return true;
    }

private:
    struct BackupData {
        vector<MetadataEntity> metadata;
        vector<FileEntity> files;
    };

    static BackupData backupExistingData(DatabaseManager& dbManager) {
        BackupData backup;

        try {
            // Try to backup metadata
            QSqlQuery metaQuery("SELECT key, value, description FROM metadata", dbManager.db);
            while (metaQuery.next()) {
                MetadataEntity meta;
                meta.key = metaQuery.value(0).toString().toStdString();
                meta.value = metaQuery.value(1).toString().toStdString();
                meta.description = metaQuery.value(2).toString().toStdString();
                backup.metadata.push_back(meta);
            }

            QSqlQuery fileQuery("SELECT file_name, file_type FROM file", dbManager.db);
            while (fileQuery.next()) {
                FileEntity file;
                file.file_name = fileQuery.value(0).toString().toStdString();
                file.file_type = fileQuery.value(1).toString().toStdString();
                backup.files.push_back(file);
            }

        } catch (const std::exception& e) {
            Logger::get().logWarning("Backup failed: " + string(e.what()));
        }

        return backup;
    }

    static bool recreateSchema(DatabaseManager& dbManager) {
        try {
            // Drop all existing tables
            QStringList tables = {"schedule_metadata", "schedule", "course", "file", "metadata"};
            for (const QString& table : tables) {
                QSqlQuery dropQuery(dbManager.db);
                dropQuery.exec("DROP TABLE IF EXISTS " + table);
                Logger::get().logInfo("Dropped table: " + table.toStdString());
            }

            // Recreate schema using schema manager
            if (!dbManager.schema()->createTables()) {
                Logger::get().logError("Failed to recreate tables");
                return false;
            }

            // Create indexes
            if (!dbManager.schema()->createIndexes()) {
                Logger::get().logWarning("Some indexes failed to create");
            }

            return true;

        } catch (const std::exception& e) {
            Logger::get().logError("Schema recreation failed: " + string(e.what()));
            return false;
        }
    }

    static bool restoreData(DatabaseManager& dbManager, const BackupData& backup) {
        bool success = true;

        // Restore metadata
        for (const auto& meta : backup.metadata) {
            if (!dbManager.insertMetadata(meta.key, meta.value, meta.description)) {
                Logger::get().logWarning("Failed to restore metadata: " + meta.key);
                success = false;
            }
        }

        // Set schema version to current
        dbManager.updateMetadata("schema_version", std::to_string(dbManager.getCurrentSchemaVersion()));
        dbManager.updateMetadata("last_repair", QDateTime::currentDateTime().toString(Qt::ISODate).toStdString());

        return success;
    }
};

bool DatabaseManager::repairDatabase() {
    Logger::get().logInfo("Attempting database repair...");
    return DatabaseRepair::repairDatabase(*this);
}

void DatabaseManager::debugDatabaseSchema() {
    if (!isConnected()) {
        Logger::get().logError("Database not connected for schema debug");
        return;
    }

    Logger::get().logInfo("=== DATABASE SCHEMA DEBUG ===");

    // List all tables
    QSqlQuery tablesQuery("SELECT name FROM sqlite_master WHERE type='table'", db);
    Logger::get().logInfo("Tables in database:");
    while (tablesQuery.next()) {
        QString tableName = tablesQuery.value(0).toString();
        Logger::get().logInfo("- " + tableName.toStdString());

        // Show columns for each table
        QSqlQuery columnsQuery("PRAGMA table_info(" + tableName + ")", db);
        while (columnsQuery.next()) {
            QString columnName = columnsQuery.value(1).toString();
            QString columnType = columnsQuery.value(2).toString();
            Logger::get().logInfo("  Column: " + columnName.toStdString() + " (" + columnType.toStdString() + ")");
        }
    }

    // List all indexes
    QSqlQuery indexesQuery("SELECT name FROM sqlite_master WHERE type='index'", db);
    Logger::get().logInfo("Indexes in database:");
    while (indexesQuery.next()) {
        QString indexName = indexesQuery.value(0).toString();
        Logger::get().logInfo("- " + indexName.toStdString());
    }

    Logger::get().logInfo("=== END SCHEMA DEBUG ===");
}

bool DatabaseManager::initializeDatabase(const QString& dbPath) {
    if (isInitialized && db.isOpen()) {
        return true;
    }

    closeDatabase();

    QString databasePath = dbPath;
    if (databasePath.isEmpty()) {
        QString appDataPath = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
        QDir appDir;
        appDir.mkpath(appDataPath);
        databasePath = QDir(appDataPath).filePath("schedulify.db");
    }

    bool isExistingDatabase = QFile::exists(databasePath);

    // Remove any existing connection
    if (QSqlDatabase::contains("schedulify_connection")) {
        QSqlDatabase::removeDatabase("schedulify_connection");
    }

    // Create new connection
    db = QSqlDatabase::addDatabase("QSQLITE", "schedulify_connection");
    db.setDatabaseName(databasePath);

    if (!db.open()) {
        Logger::get().logError("Failed to open database: " + db.lastError().text().toStdString());
        return false;
    }

    Logger::get().logInfo("Database opened successfully");

    // Initialize managers
    schemaManager = std::make_unique<DatabaseSchema>(db);
    fileManager = std::make_unique<DatabaseFileManager>(db);
    courseManager = std::make_unique<DatabaseCourseManager>(db);
    scheduleManager = std::make_unique<DatabaseScheduleManager>(db);

    // Try to validate existing database first
    bool needsSchemaCreation = false;
    bool needsSchemaUpgrade = false;
    int currentSchemaVersion = 0;

    if (isExistingDatabase) {
        // Check if metadata table exists
        QSqlQuery checkQuery("SELECT name FROM sqlite_master WHERE type='table' AND name='metadata'", db);
        if (!checkQuery.exec() || !checkQuery.next()) {
            Logger::get().logWarning("Metadata table not found - database needs initialization");
            needsSchemaCreation = true;
        } else {
            // Check schema version
            QSqlQuery versionQuery("SELECT value FROM metadata WHERE key = 'schema_version'", db);
            if (versionQuery.exec() && versionQuery.next()) {
                string version = versionQuery.value(0).toString().toStdString();
                currentSchemaVersion = std::stoi(version);

                if (currentSchemaVersion < getCurrentSchemaVersion()) {
                    needsSchemaUpgrade = true;
                } else if (currentSchemaVersion > getCurrentSchemaVersion()) {
                    Logger::get().logError("Database schema version is newer than supported - cannot proceed");
                    closeDatabase();
                    return false;
                }
            } else {
                Logger::get().logWarning("No schema version found - needs initialization");
                needsSchemaCreation = true;
            }
        }

        // If existing database seems valid, try to validate full schema
        if (!needsSchemaCreation && !needsSchemaUpgrade) {
            if (!schemaManager->validateSchema()) {
                Logger::get().logWarning("Schema validation failed - will recreate tables");
                needsSchemaCreation = true;
            } else {
                Logger::get().logInfo("Existing database validated successfully");
            }
        }
    } else {
        Logger::get().logInfo("No existing database - will create new one");
        needsSchemaCreation = true;
    }

    // Handle schema upgrade
    if (needsSchemaUpgrade) {
        if (!schemaManager->upgradeSchema(currentSchemaVersion, getCurrentSchemaVersion())) {
            Logger::get().logError("Schema upgrade failed - will recreate database");
            needsSchemaCreation = true;
            needsSchemaUpgrade = false;
        } else {
            updateMetadata("schema_version", std::to_string(getCurrentSchemaVersion()));
        }
    }

    // Create or recreate schema if needed
    if (needsSchemaCreation) {

        // Create tables using schema manager
        if (!schemaManager->createTables()) {
            Logger::get().logError("Failed to create database tables");
            closeDatabase();
            return false;
        }

        // Insert initial metadata
        insertMetadata("schema_version", std::to_string(getCurrentSchemaVersion()), "Database schema version");
        insertMetadata("created_at", QDateTime::currentDateTime().toString(Qt::ISODate).toStdString(), "Database creation timestamp");

        Logger::get().logInfo("Database schema created successfully");
    }

    // Create indexes (safe to call multiple times)
    if (!schemaManager->createIndexes()) {
        Logger::get().logWarning("Some indexes failed to create - continuing anyway");
    }

    // Test write capability
    QSqlQuery writeTest(db);
    if (!writeTest.exec("CREATE TEMP TABLE write_test (id INTEGER)") ||
        !writeTest.exec("INSERT INTO write_test (id) VALUES (1)") ||
        !writeTest.exec("DROP TABLE write_test")) {
        Logger::get().logError("Database write test failed: " + writeTest.lastError().text().toStdString());
        closeDatabase();
        return false;
    }

    // Update last access time
    updateMetadata("last_access", QDateTime::currentDateTime().toString(Qt::ISODate).toStdString());

    isInitialized = true;

    return true;
}

bool DatabaseManager::isConnected() const {
    return isInitialized && db.isOpen();
}

void DatabaseManager::closeDatabase() {
    // Reset managers before closing database
    scheduleManager.reset();
    courseManager.reset();
    fileManager.reset();
    schemaManager.reset();

    if (db.isOpen()) {
        db.close();
    }
    QSqlDatabase::removeDatabase("schedulify_connection");
    isInitialized = false;
}

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
    return false;
}

bool DatabaseManager::importFromFile(const QString& filePath) {
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