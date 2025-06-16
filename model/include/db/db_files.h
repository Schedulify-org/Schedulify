#ifndef DB_FILES_H
#define DB_FILES_H

#include "db_entities.h"
#include "logger.h"

#include <QSqlQuery>
#include <QSqlError>
#include <QSqlRecord>
#include <QVariant>
#include <QSqlDatabase>
#include <vector>
#include <string>

using namespace std;

class DatabaseFileManager {
public:
    explicit DatabaseFileManager(QSqlDatabase& database);

    // File CRUD operations
    int insertFile(const string& fileName, const string& fileType);  // Returns file ID
    bool updateFile(int fileId, const string& fileName, const string& fileType);
    bool deleteFile(int fileId);
    bool deleteAllFiles();

    // File retrieval operations
    vector<FileEntity> getAllFiles();
    FileEntity getFileById(int id);
    FileEntity getFileByName(const string& fileName);  // Gets most recent by upload_time
    int getFileIdByName(const string& fileName);       // Gets most recent by upload_time

    // File utility operations
    bool fileExists(int fileId);
    int getFileCount();
    QDateTime getFileUploadTime(int fileId);

    // File statistics
    vector<FileEntity> getFilesByType(const string& fileType);
    vector<FileEntity> getRecentFiles(int limit = 10);

private:
    QSqlDatabase& db;

    // Helper methods
    FileEntity createFileEntityFromQuery(QSqlQuery& query);
};

#endif // DB_FILES_H