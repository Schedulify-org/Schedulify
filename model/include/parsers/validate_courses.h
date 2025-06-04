#ifndef VALIDATE_COURSES_H
#define VALIDATE_COURSES_H
#include "model_interfaces.h"
#include <vector>
#include <algorithm>

struct Slot{
  string start_time;
  string end_time;
  string id;
};
struct Day_of_week{
  int id;
  vector<Slot> Slot;
};
struct Room{
  string id;
  vector<Day_of_week> Day_of_week;
  bool operator==(const std::string& s) const {
    return id == s;
  }
};
struct Building{
  string id;
  vector<Room> Room ;
  bool operator==(const std::string& s) const {
    return id == s;
  }
};
int toMinutes(const std::string& t);
bool isOverLapping(const Slot &s1, const Slot &s2);

vector<string> validate_courses(vector<Course> courses);
#endif //VALIDATE_COURSES_H
