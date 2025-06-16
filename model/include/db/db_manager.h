#ifndef DATABASE_MANAGER_H
#define DATABASE_MANAGER_H

#include "db_entities.h"
#include "db_schema.h"
#include "db_files.h"
#include "db_courses.h"
#include "logger.h"

#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlError>
#include <QStandardPaths>
#include <QDir>
#include <QString>
#include <QVariant>
#include <QDateTime>
#include <vector>
#include <string>
#include <memory>

using namespace std;

class DatabaseManager {
public:
    static DatabaseManager& getInstance();

    // Database lifecycle
    bool initializeDatabase(const QString& dbPath = QString());
    bool isConnected() const;
    void closeDatabase();

    // Manager access - providing delegation to specialized managers
    DatabaseFileManager* files() { return fileManager.get(); }
    DatabaseCourseManager* courses() { return courseManager.get(); }
    DatabaseSchema* schema() { return schemaManager.get(); }

    // Metadata operations (kept in main manager for simplicity)
    bool insertMetadata(const string& key, const string& value, const string& description = "");
    bool updateMetadata(const string& key, const string& value);
    string getMetadata(const string& key, const string& defaultValue = "");
    vector<MetadataEntity> getAllMetadata();

    // Utility operations
    bool clearAllData();
    int getTableRowCount(const string& tableName);

    // Database repair and debugging
    bool repairDatabase();
    void debugDatabaseSchema();

    // Transaction support
    bool beginTransaction();
    bool commitTransaction();
    bool rollbackTransaction();

    // Schema version access
    static int getCurrentSchemaVersion() { return CURRENT_SCHEMA_VERSION; }

private:
    DatabaseManager() = default;
    ~DatabaseManager();

    // Disable copy/move
    DatabaseManager(const DatabaseManager&) = delete;
    DatabaseManager& operator=(const DatabaseManager&) = delete;
    DatabaseManager(DatabaseManager&&) = delete;
    DatabaseManager& operator=(DatabaseManager&&) = delete;

    // Internal methods
    bool executeQuery(const QString& query, const QVariantList& params = QVariantList());

    // Database connection and managers
    QSqlDatabase db;
    bool isInitialized = false;

    // Specialized managers
    std::unique_ptr<DatabaseSchema> schemaManager;
    std::unique_ptr<DatabaseFileManager> fileManager;
    std::unique_ptr<DatabaseCourseManager> courseManager;

    // Database schema version for migrations
    static const int CURRENT_SCHEMA_VERSION = 2;

    // Friend class for repair operations
    friend class DatabaseRepair;
};

class DatabaseTransaction {
public:
    explicit DatabaseTransaction(DatabaseManager& dbManager);
    ~DatabaseTransaction();

    bool commit();
    void rollback();

private:
    DatabaseManager& db;
    bool committed = false;
    bool rolledBack = false;
};

#endif // DATABASE_MANAGER_H