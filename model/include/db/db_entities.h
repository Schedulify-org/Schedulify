#ifndef DB_ENTITIES_H
#define DB_ENTITIES_H

#include <QDateTime>
#include <string>
#include <vector>

using namespace std;

// Database entity structures
struct FileEntity {
    int id = 0;
    string file_name;
    string file_type;         // "txt", "xlsx", etc.
    QDateTime upload_time;    // When the file was uploaded
    QDateTime updated_at;     // Last update time
};

struct CourseEntity {
    int id = 0;
    string raw_id;
    string name;
    string teacher;
    string lectures_json;     // JSON string for lectures groups
    string tutorials_json;    // JSON string for tutorials groups
    string labs_json;         // JSON string for labs groups
    string blocks_json;       // JSON string for blocks groups
    int file_id = 0;         // Foreign key to file table - REQUIRED
    QDateTime created_at;
    QDateTime updated_at;
};

struct ScheduleEntity {
    int id = 0;
    int schedule_index = 0;
    string courses_json;      // JSON string of course IDs used
    string week_json;         // JSON string of the complete week schedule
    int amount_days = 0;
    int amount_gaps = 0;
    int gaps_time = 0;
    int avg_start = 0;
    int avg_end = 0;
    QDateTime created_at;
};

struct ScheduleMetadataEntity {
    int id = 0;
    int total_schedules = 0;
    string generation_settings_json;  // JSON of generation parameters
    QDateTime generated_at;
    string status;            // "active", "archived", etc.
};

struct MetadataEntity {
    int id = 0;
    string key;
    string value;
    string description;
    QDateTime updated_at;
};

#endif // DB_ENTITIES_H