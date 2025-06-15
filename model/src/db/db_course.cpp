#include "db_courses.h"
#include "db_json_helpers.h"
#include "logger.h"
#include <QSqlQuery>
#include <QSqlError>
#include <QVariant>
#include <algorithm>

DatabaseCourseManager::DatabaseCourseManager(QSqlDatabase& database) : db(database) {
}

bool DatabaseCourseManager::insertCourse(const Course& course, int fileId) {
    if (!db.isOpen()) {
        Logger::get().logError("Database not open for course insertion");
        return false;
    }

    if (fileId <= 0) {
        Logger::get().logError("Cannot insert course without valid file ID");
        return false;
    }

    QSqlQuery query(db);
    query.prepare(R"(
        INSERT OR REPLACE INTO course
        (course_file_id, raw_id, name, teacher, lectures_json, tutorials_json, labs_json, blocks_json, file_id, updated_at)
        VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?, CURRENT_TIMESTAMP)
    )");

    query.addBindValue(course.id);  // This goes to course_file_id
    query.addBindValue(QString::fromStdString(course.raw_id));
    query.addBindValue(QString::fromStdString(course.name));
    query.addBindValue(QString::fromStdString(course.teacher));
    query.addBindValue(QString::fromStdString(DatabaseJsonHelpers::groupsToJson(course.Lectures)));
    query.addBindValue(QString::fromStdString(DatabaseJsonHelpers::groupsToJson(course.Tirgulim)));
    query.addBindValue(QString::fromStdString(DatabaseJsonHelpers::groupsToJson(course.labs)));
    query.addBindValue(QString::fromStdString(DatabaseJsonHelpers::groupsToJson(course.blocks)));
    query.addBindValue(fileId);

    if (!query.exec()) {
        Logger::get().logError("Failed to insert course: " + query.lastError().text().toStdString());
        Logger::get().logError("Course details - File ID: " + std::to_string(course.id) +
                               ", Raw ID: " + course.raw_id + ", Name: " + course.name);
        return false;
    }

    return true;
}

bool DatabaseCourseManager::insertCourses(const vector<Course>& courses, int fileId) {
    if (!db.isOpen()) {
        Logger::get().logError("Database not open for course insertion");
        return false;
    }

    if (fileId <= 0) {
        Logger::get().logError("Cannot insert courses without valid file ID");
        return false;
    }

    if (courses.empty()) {
        Logger::get().logWarning("No courses to insert");
        return true;
    }

    Logger::get().logInfo("Inserting " + std::to_string(courses.size()) + " courses for file ID: " + std::to_string(fileId));

    // Start transaction
    if (!db.transaction()) {
        Logger::get().logError("Failed to begin transaction for course insertion");
        return false;
    }

    int successCount = 0;
    for (const auto& course : courses) {
        if (insertCourse(course, fileId)) {
            successCount++;
        } else {
            Logger::get().logError("Failed to insert course: " + course.name + " (File ID: " + std::to_string(course.id) + ")");
            // Continue with other courses rather than aborting entire transaction
        }
    }

    if (successCount == 0) {
        Logger::get().logError("Failed to insert any courses, rolling back transaction");
        db.rollback();
        return false;
    }

    if (!db.commit()) {
        Logger::get().logError("Failed to commit course insertion transaction");
        db.rollback();
        return false;
    }

    Logger::get().logInfo("Successfully inserted " + std::to_string(successCount) + "/" +
                          std::to_string(courses.size()) + " courses with file ID: " + std::to_string(fileId));

    return successCount == static_cast<int>(courses.size());
}

bool DatabaseCourseManager::updateCourse(const Course& course, int fileId) {
    return insertCourse(course, fileId); // Uses INSERT OR REPLACE
}

bool DatabaseCourseManager::deleteCourse(int courseId) {
    if (!db.isOpen()) {
        Logger::get().logError("Database not open for course deletion");
        return false;
    }

    QSqlQuery query(db);
    query.prepare("DELETE FROM course WHERE id = ?");
    query.addBindValue(courseId);

    if (!query.exec()) {
        Logger::get().logError("Failed to delete course: " + query.lastError().text().toStdString());
        return false;
    }

    int rowsAffected = query.numRowsAffected();
    if (rowsAffected == 0) {
        Logger::get().logWarning("No course found with ID: " + std::to_string(courseId));
        return false;
    }

    Logger::get().logInfo("Course deleted successfully (ID: " + std::to_string(courseId) + ")");
    return true;
}

bool DatabaseCourseManager::deleteAllCourses() {
    if (!db.isOpen()) {
        Logger::get().logError("Database not open for course deletion");
        return false;
    }

    QSqlQuery query("DELETE FROM course", db);
    if (!query.exec()) {
        Logger::get().logError("Failed to delete all courses: " + query.lastError().text().toStdString());
        return false;
    }

    int rowsAffected = query.numRowsAffected();
    Logger::get().logInfo("Deleted all courses from database (" + std::to_string(rowsAffected) + " courses)");
    return true;
}

bool DatabaseCourseManager::deleteCoursesByFileId(int fileId) {
    if (!db.isOpen()) {
        Logger::get().logError("Database not open for course deletion");
        return false;
    }

    QSqlQuery query(db);
    query.prepare("DELETE FROM course WHERE file_id = ?");
    query.addBindValue(fileId);

    if (!query.exec()) {
        Logger::get().logError("Failed to delete courses for file ID " + std::to_string(fileId) +
                               ": " + query.lastError().text().toStdString());
        return false;
    }

    int deletedCount = query.numRowsAffected();
    Logger::get().logInfo("Deleted " + std::to_string(deletedCount) + " courses for file ID: " + std::to_string(fileId));
    return true;
}

vector<Course> DatabaseCourseManager::getAllCourses() {
    vector<Course> courses;
    if (!db.isOpen()) {
        Logger::get().logError("Database not open for course retrieval");
        return courses;
    }

    QSqlQuery query(R"(
        SELECT id, course_file_id, raw_id, name, teacher, lectures_json, tutorials_json, labs_json, blocks_json, file_id
        FROM course ORDER BY course_file_id
    )", db);

    while (query.next()) {
        courses.push_back(createCourseFromQuery(query));
    }

    Logger::get().logInfo("Retrieved " + std::to_string(courses.size()) + " courses from database");
    return courses;
}

Course DatabaseCourseManager::getCourseById(int id) {
    Course course;
    if (!db.isOpen()) {
        Logger::get().logError("Database not open for course retrieval");
        return course;
    }

    QSqlQuery query(db);
    query.prepare(R"(
        SELECT id, course_file_id, raw_id, name, teacher, lectures_json, tutorials_json, labs_json, blocks_json, file_id
        FROM course WHERE id = ?
    )");
    query.addBindValue(id);

    if (query.exec() && query.next()) {
        course = createCourseFromQuery(query);
    } else {
        Logger::get().logWarning("No course found with ID: " + std::to_string(id));
    }

    return course;
}

vector<Course> DatabaseCourseManager::getCoursesByName(const string& name) {
    vector<Course> courses;
    if (!db.isOpen()) {
        return courses;
    }

    QSqlQuery query(db);
    query.prepare(R"(
        SELECT id, course_file_id, raw_id, name, teacher, lectures_json, tutorials_json, labs_json, blocks_json, file_id
        FROM course WHERE name LIKE ? ORDER BY name
    )");
    query.addBindValue("%" + QString::fromStdString(name) + "%");

    while (query.next()) {
        courses.push_back(createCourseFromQuery(query));
    }

    return courses;
}

vector<Course> DatabaseCourseManager::getCoursesByFileId(int fileId) {
    vector<Course> courses;
    if (!db.isOpen()) {
        Logger::get().logError("Database not open for course retrieval");
        return courses;
    }

    QSqlQuery query(db);
    query.prepare(R"(
        SELECT id, course_file_id, raw_id, name, teacher, lectures_json, tutorials_json, labs_json, blocks_json, file_id
        FROM course WHERE file_id = ? ORDER BY course_file_id
    )");
    query.addBindValue(fileId);

    int courseCount = 0;
    while (query.next()) {
        courses.push_back(createCourseFromQuery(query));
        courseCount++;
    }

    Logger::get().logInfo("Found " + std::to_string(courseCount) + " courses for file ID: " + std::to_string(fileId));

    // Debug: Log first few courses
    for (size_t i = 0; i < std::min(courses.size(), size_t(3)); ++i) {
        Logger::get().logInfo("  Course " + std::to_string(i) + ": File ID=" + std::to_string(courses[i].id) +
                              ", Raw ID=" + courses[i].raw_id + ", Name=" + courses[i].name);
    }

    return courses;
}

vector<Course> DatabaseCourseManager::getCoursesByFileIds(const vector<int>& fileIds, vector<string>& warnings) {
    warnings.clear();

    if (!db.isOpen() || fileIds.empty()) {
        Logger::get().logError("Database not open or no file IDs provided");
        return {};
    }

    Logger::get().logInfo("=== COURSE RETRIEVAL BY FILE IDS DEBUG ===");
    Logger::get().logInfo("Requested file IDs count: " + std::to_string(fileIds.size()));

    map<string, vector<CourseConflictInfo>> conflictMap;

    for (int fileId : fileIds) {
        Logger::get().logInfo("Processing file ID: " + std::to_string(fileId));

        QSqlQuery query(db);
        query.prepare(R"(
            SELECT c.id, c.course_file_id, c.raw_id, c.name, c.teacher, c.lectures_json, c.tutorials_json,
                   c.labs_json, c.blocks_json, c.file_id, f.file_name, f.upload_time
            FROM course c
            JOIN file f ON c.file_id = f.id
            WHERE c.file_id = ?
            ORDER BY f.upload_time ASC
        )");
        query.addBindValue(fileId);

        if (!query.exec()) {
            Logger::get().logError("Failed to execute query for file ID " + std::to_string(fileId) +
                                   ": " + query.lastError().text().toStdString());
            continue;
        }

        int courseCount = 0;
        while (query.next()) {
            Course course = createCourseFromQuery(query);
            string fileName = query.value(10).toString().toStdString();
            QDateTime uploadTime = query.value(11).toDateTime();

            Logger::get().logInfo("  Found course: File ID=" + std::to_string(course.id) +
                                  ", Raw ID=" + course.raw_id + ", Name=" + course.name +
                                  ", From file: " + fileName);

            CourseConflictInfo conflictInfo;
            conflictInfo.course = course;
            conflictInfo.uploadTime = uploadTime;
            conflictInfo.fileName = fileName;
            conflictInfo.fileId = fileId;

            conflictMap[course.raw_id].push_back(conflictInfo);
            courseCount++;
        }

        Logger::get().logInfo("File ID " + std::to_string(fileId) + " contributed " + std::to_string(courseCount) + " courses");
    }

    Logger::get().logInfo("Total unique course raw_ids found: " + std::to_string(conflictMap.size()));

    return resolveConflicts(conflictMap, warnings);
}

vector<Course> DatabaseCourseManager::getCoursesByRawId(const string& rawId) {
    vector<Course> courses;
    if (!db.isOpen()) {
        return courses;
    }

    QSqlQuery query(db);
    query.prepare(R"(
        SELECT id, course_file_id, raw_id, name, teacher, lectures_json, tutorials_json, labs_json, blocks_json, file_id
        FROM course WHERE raw_id = ? ORDER BY course_file_id
    )");
    query.addBindValue(QString::fromStdString(rawId));

    while (query.next()) {
        courses.push_back(createCourseFromQuery(query));
    }

    return courses;
}

bool DatabaseCourseManager::courseExists(int courseId) {
    if (!db.isOpen()) {
        return false;
    }

    QSqlQuery query(db);
    query.prepare("SELECT COUNT(*) FROM course WHERE id = ?");
    query.addBindValue(courseId);

    if (query.exec() && query.next()) {
        return query.value(0).toInt() > 0;
    }

    return false;
}

bool DatabaseCourseManager::courseExistsByRawId(const string& rawId) {
    if (!db.isOpen()) {
        return false;
    }

    QSqlQuery query(db);
    query.prepare("SELECT COUNT(*) FROM course WHERE raw_id = ?");
    query.addBindValue(QString::fromStdString(rawId));

    if (query.exec() && query.next()) {
        return query.value(0).toInt() > 0;
    }

    return false;
}

bool DatabaseCourseManager::courseExistsByRawIdAndFileId(const string& rawId, int fileId) {
    if (!db.isOpen()) {
        return false;
    }

    QSqlQuery query(db);
    query.prepare("SELECT COUNT(*) FROM course WHERE raw_id = ? AND file_id = ?");
    query.addBindValue(QString::fromStdString(rawId));
    query.addBindValue(fileId);

    if (query.exec() && query.next()) {
        return query.value(0).toInt() > 0;
    }

    return false;
}

int DatabaseCourseManager::getCourseCount() {
    if (!db.isOpen()) {
        return -1;
    }

    QSqlQuery query("SELECT COUNT(*) FROM course", db);

    if (query.exec() && query.next()) {
        return query.value(0).toInt();
    }

    return -1;
}

int DatabaseCourseManager::getCourseCountByFileId(int fileId) {
    if (!db.isOpen()) {
        return -1;
    }

    QSqlQuery query(db);
    query.prepare("SELECT COUNT(*) FROM course WHERE file_id = ?");
    query.addBindValue(fileId);

    if (query.exec() && query.next()) {
        return query.value(0).toInt();
    }

    return -1;
}

QDateTime DatabaseCourseManager::getCourseCreationTime(int courseId) {
    if (!db.isOpen()) {
        return QDateTime();
    }

    QSqlQuery query(db);
    query.prepare("SELECT created_at FROM course WHERE id = ?");
    query.addBindValue(courseId);

    if (query.exec() && query.next()) {
        return query.value(0).toDateTime();
    }

    return QDateTime();
}

map<string, int> DatabaseCourseManager::getCourseCountsByType() {
    map<string, int> counts;
    if (!db.isOpen()) {
        return counts;
    }

    QSqlQuery query(R"(
        SELECT f.file_type, COUNT(c.id) as course_count
        FROM course c
        JOIN file f ON c.file_id = f.id
        GROUP BY f.file_type
    )", db);

    while (query.next()) {
        string fileType = query.value(0).toString().toStdString();
        int count = query.value(1).toInt();
        counts[fileType] = count;
    }

    return counts;
}

vector<Course> DatabaseCourseManager::getCoursesCreatedAfter(const QDateTime& dateTime) {
    vector<Course> courses;
    if (!db.isOpen()) {
        return courses;
    }

    QSqlQuery query(db);
    query.prepare(R"(
        SELECT id, course_file_id, raw_id, name, teacher, lectures_json, tutorials_json, labs_json, blocks_json, file_id
        FROM course WHERE created_at > ? ORDER BY created_at DESC
    )");
    query.addBindValue(dateTime);

    while (query.next()) {
        courses.push_back(createCourseFromQuery(query));
    }

    return courses;
}

vector<Course> DatabaseCourseManager::getCoursesByTeacher(const string& teacher) {
    vector<Course> courses;
    if (!db.isOpen()) {
        return courses;
    }

    QSqlQuery query(db);
    query.prepare(R"(
        SELECT id, course_file_id, raw_id, name, teacher, lectures_json, tutorials_json, labs_json, blocks_json, file_id
        FROM course WHERE teacher LIKE ? ORDER BY name
    )");
    query.addBindValue("%" + QString::fromStdString(teacher) + "%");

    while (query.next()) {
        courses.push_back(createCourseFromQuery(query));
    }

    return courses;
}

bool DatabaseCourseManager::executeQuery(const QString& query, const QVariantList& params) {
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

Course DatabaseCourseManager::createCourseFromQuery(QSqlQuery& query) {
    Course course;

    // Query returns: id (auto-increment), course_file_id, raw_id, name, teacher, lectures_json, tutorials_json, labs_json, blocks_json, file_id
    int dbId = query.value(0).toInt();  // Auto-increment ID from database (not used in course object)
    course.id = query.value(1).toInt();  // Use course_file_id as the course.id
    course.raw_id = query.value(2).toString().toStdString();
    course.name = query.value(3).toString().toStdString();
    course.teacher = query.value(4).toString().toStdString();
    course.Lectures = DatabaseJsonHelpers::groupsFromJson(query.value(5).toString().toStdString());
    course.Tirgulim = DatabaseJsonHelpers::groupsFromJson(query.value(6).toString().toStdString());
    course.labs = DatabaseJsonHelpers::groupsFromJson(query.value(7).toString().toStdString());
    course.blocks = DatabaseJsonHelpers::groupsFromJson(query.value(8).toString().toStdString());

    return course;
}

vector<Course> DatabaseCourseManager::resolveConflicts(const map<string, vector<CourseConflictInfo>>& conflictMap,
                                                       vector<string>& warnings) {
    vector<Course> courses;

    Logger::get().logInfo("=== RESOLVING COURSE CONFLICTS ===");

    for (const auto& pair : conflictMap) {
        const string& rawId = pair.first;
        const vector<CourseConflictInfo>& conflicts = pair.second;

        if (conflicts.size() == 1) {
            // No conflict, just add the course
            courses.push_back(conflicts[0].course);
            Logger::get().logInfo("No conflict for raw_id: " + rawId);
        } else {
            // Multiple courses with same raw_id, resolve by upload time
            auto latest = std::max_element(conflicts.begin(), conflicts.end(),
                                           [](const CourseConflictInfo& a, const CourseConflictInfo& b) {
                                               return a.uploadTime < b.uploadTime;
                                           });

            courses.push_back(latest->course);

            // Generate warning message
            string warningMsg = "Course conflict resolved for " + rawId + " - using version from " + latest->fileName + " (latest upload)";
            warnings.push_back(warningMsg);

            Logger::get().logWarning(warningMsg);
            Logger::get().logInfo("Conflict details for " + rawId + ":");
            for (const auto& conflict : conflicts) {
                Logger::get().logInfo("  - File: " + conflict.fileName + ", Upload: " +
                                      conflict.uploadTime.toString("yyyy-MM-dd hh:mm:ss").toStdString() +
                                      (conflict.uploadTime == latest->uploadTime ? " [SELECTED]" : ""));
            }
        }
    }

    Logger::get().logInfo("=== CONFLICT RESOLUTION COMPLETE ===");
    Logger::get().logInfo("Final result: " + std::to_string(courses.size()) + " courses with " +
                          std::to_string(warnings.size()) + " conflict(s) resolved");

    return courses;
}