#ifndef DB_ENTITIES_H
#define DB_ENTITIES_H

#include <QDateTime>
#include <string>
#include <vector>

using namespace std;

// Database entity structures representing table schemas
struct FileEntity {
    int id = 0;                   // Primary key
    string file_name;             // Original filename
    string file_type;             // File extension (txt, xlsx, etc.)
    QDateTime upload_time;        // When the file was first uploaded
    QDateTime updated_at;         // Last modification time

    // Default constructor
    FileEntity() = default;

    // Constructor for easy creation
    FileEntity(const string& name, const string& type)
            : file_name(name), file_type(type),
              upload_time(QDateTime::currentDateTime()),
              updated_at(QDateTime::currentDateTime()) {}
};

struct CourseEntity {
    int id = 0;                   // Course ID (matches Course.id)
    string raw_id;                // Original course identifier from file
    string name;                  // Course name
    string teacher;               // Teacher/instructor name
    string lectures_json;         // JSON serialized lectures groups
    string tutorials_json;        // JSON serialized tutorials groups
    string labs_json;             // JSON serialized labs groups
    string blocks_json;           // JSON serialized blocks groups
    int file_id = 0;             // Foreign key to file table - REQUIRED
    QDateTime created_at;         // When course was added to database
    QDateTime updated_at;         // Last modification time

    // Default constructor
    CourseEntity() = default;

    // Constructor for database insertion
    CourseEntity(int courseId, const string& rawId, const string& courseName,
                 const string& teacherName, int fileId)
            : id(courseId), raw_id(rawId), name(courseName), teacher(teacherName),
              file_id(fileId), created_at(QDateTime::currentDateTime()),
              updated_at(QDateTime::currentDateTime()) {}
};

struct ScheduleEntity {
    int id = 0;                   // Primary key (auto-increment)
    int schedule_index = 0;       // Schedule index from InformativeSchedule
    string courses_json;          // JSON array of course IDs used in this schedule
    string week_json;             // JSON serialized complete week schedule
    int amount_days = 0;          // Number of days with classes
    int amount_gaps = 0;          // Number of gaps in schedule
    int gaps_time = 0;            // Total gap time in minutes
    int avg_start = 0;            // Average start time (minutes from midnight)
    int avg_end = 0;              // Average end time (minutes from midnight)
    QDateTime created_at;         // When schedule was generated

    // Default constructor
    ScheduleEntity() = default;

    // Constructor for database insertion
    ScheduleEntity(int index, const string& coursesJson, const string& weekJson,
                   int days, int gaps, int gapsTime, int avgStart, int avgEnd)
            : schedule_index(index), courses_json(coursesJson), week_json(weekJson),
              amount_days(days), amount_gaps(gaps), gaps_time(gapsTime),
              avg_start(avgStart), avg_end(avgEnd),
              created_at(QDateTime::currentDateTime()) {}
};

struct ScheduleMetadataEntity {
    int id = 0;                   // Primary key (auto-increment)
    int total_schedules = 0;      // Total number of schedules generated
    string generation_settings_json; // JSON of generation parameters and settings
    QDateTime generated_at;       // When this batch of schedules was generated
    string status;                // Status: "active", "archived", "deleted", etc.

    // Default constructor
    ScheduleMetadataEntity() = default;

    // Constructor for database insertion
    ScheduleMetadataEntity(int totalSchedules, const string& settings)
            : total_schedules(totalSchedules), generation_settings_json(settings),
              generated_at(QDateTime::currentDateTime()), status("active") {}
};

struct MetadataEntity {
    int id = 0;                   // Primary key (auto-increment)
    string key;                   // Metadata key (unique)
    string value;                 // Metadata value
    string description;           // Human-readable description of the metadata
    QDateTime updated_at;         // When this metadata was last updated

    // Default constructor
    MetadataEntity() = default;

    // Constructor for database insertion
    MetadataEntity(const string& metaKey, const string& metaValue,
                   const string& desc = "")
            : key(metaKey), value(metaValue), description(desc),
              updated_at(QDateTime::currentDateTime()) {}
};

// Helper structure for database statistics
struct DatabaseStatistics {
    int total_files = 0;
    int total_courses = 0;
    int total_schedules = 0;
    int total_metadata_entries = 0;
    string database_version;
    QDateTime last_accessed;
    QString database_size;

    // Default constructor
    DatabaseStatistics() = default;
};

// Helper structure for course conflict resolution
struct CourseConflictInfo {
    int course_id = 0;
    string raw_id;
    string course_name;
    int file_id = 0;
    string file_name;
    QDateTime upload_time;

    // Default constructor
    CourseConflictInfo() = default;

    // Constructor for conflict tracking
    CourseConflictInfo(int courseId, const string& rawId, const string& name,
                       int fileId, const string& fileName, const QDateTime& uploadTime)
            : course_id(courseId), raw_id(rawId), course_name(name),
              file_id(fileId), file_name(fileName), upload_time(uploadTime) {}
};

#endif // DB_ENTITIES_H