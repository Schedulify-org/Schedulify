#ifndef MODEL_INTERFACES_H
#define MODEL_INTERFACES_H

#include <string>
#include <vector>

using namespace std;
class Session {
public:
    int day_of_week;        // יום בשבוע בין 1-7 (ראשון - ראשון)
    string start_time;      // שעת התחלה של הלוח הזמנים
    string end_time;        // שעת הסיום של הלוח הזמנים
    string building_number; // מספר הבנין בו מתקיים המתוכן המופע
    string room_number;     // מספר החדר בו מתקיים המתוכן המופע
};

class Group {
public:
    string type;                    // סוג הקבוצה: Lecture / tutorial / lab
    vector<Session> sessions;       // הקבוצה חייבת להכיל לפחות session אחד
};

class Course {
public:
    int id;
    string raw_id; // מזהה (קוד) של קורס
    string name;                    // שם הקורס
    string teacher;                 // שם המרצה
    vector<Group> Lectures;         // רשימת הרצאות - נדרש לפחות קבוצה אחת עם הרצאה 1
    vector<Group> Tirgulim;         // רשימת תרגולים - יכול להיות ריק
    vector<Group> labs;
};


struct ScheduleItem {
    string courseName;
    string raw_id;
    string type;
    string start;
    string end;
    string building;
    string room;
};

struct ScheduleDay {
    string day;
    vector<ScheduleItem> day_items;
};

struct InformativeSchedule {
    int index;
    vector<ScheduleDay> week;
};

enum class ModelOperation {
    GENERATE_COURSES,
    GENERATE_SCHEDULES,
    SAVE_SCHEDULE,
    PRINT_SCHEDULE
};

class IModel {
public:
    virtual ~IModel() = default;
    virtual void* executeOperation(ModelOperation operation, const void* data, const std::string& path) = 0;
};

#endif //MODEL_INTERFACES_H
