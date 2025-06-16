#ifndef DB_SCHEMA_H
#define DB_SCHEMA_H

#include "logger.h"

#include <QSqlDatabase>
#include <QString>
#include <QSqlQuery>
#include <QSqlError>

class DatabaseSchema {
public:
    explicit DatabaseSchema(QSqlDatabase& database);

    // Schema management
    bool createTables();
    bool createIndexes();
    bool dropAllTables();

    // Schema versioning
    static int getCurrentSchemaVersion() { return CURRENT_SCHEMA_VERSION; }
    bool upgradeSchema(int fromVersion, int toVersion);

    // Validation
    bool validateSchema();
    bool tableExists(const QString& tableName);

private:
    QSqlDatabase& db;
    static const int CURRENT_SCHEMA_VERSION = 2;

    // Individual table creation methods
    bool createMetadataTable();
    bool createFileTable();
    bool createCourseTable();

    // Index creation methods
    bool createFileIndexes();
    bool createCourseIndexes();
    bool createMetadataIndexes();

    // Schema upgrade methods
    bool upgradeFromV1ToV2();

    // Validation methods
    bool validateFileTableColumns();
    bool validateCourseTableColumns();

    // Utility methods
    bool executeQuery(const QString& query);
};

#endif // DB_SCHEMA_H