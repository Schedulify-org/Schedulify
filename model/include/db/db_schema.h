#ifndef DB_SCHEMA_H
#define DB_SCHEMA_H

#include <QSqlDatabase>
#include <QString>

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
    static const int CURRENT_SCHEMA_VERSION = 3;  // Updated to version 3

    // Individual table creation methods
    bool createMetadataTable();
    bool createFileTable();
    bool createCourseTable();
    bool createScheduleTable();
    bool createScheduleMetadataTable();

    // Index creation methods
    bool createFileIndexes();
    bool createCourseIndexes();
    bool createScheduleIndexes();
    bool createMetadataIndexes();

    // Schema upgrade methods
    bool upgradeFromV1ToV2();
    bool upgradeFromV2ToV3();  // New upgrade method

    // Validation methods
    bool validateFileTableColumns();
    bool validateCourseTableColumns();  // New validation method

    // Utility methods
    bool executeQuery(const QString& query);
};

#endif // DB_SCHEMA_H