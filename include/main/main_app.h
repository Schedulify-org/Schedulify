#ifndef SCHED_MAIN_APP_H
#define SCHED_MAIN_APP_H

#include "parsers/parseCoursesToVector.h"
#include "parsers/parseSchedToJson.h"
#include "parsers/parseCoursesToJson.h"
#include "schedule_algorithm/ScheduleBuilder.h"

#include <vector>
#include <map>
#include <string>
#include <iostream>

using std::string;
using std::cout;
using std::vector;
using std::map;

// main function to load data file, generate schedules and export as json
int main_app(const string& inputPath, const string& modifiedOutputPath, const string& courseOutput ,const string& userInput);

int app_main(const string& action_selected);

class ICommand {
public:
    virtual bool execute(string answer) = 0;
};

//main menu initiate, connect function to commend
map<string, ICommand*> initiate_main_menu();

//checks if a given commend is in commend menu
bool keyExistsInMap(const map<string, ICommand *>& commands, const string& key);

//Generate Course File class for main menu
class GenerateCourseFile : public ICommand {
public:
    //run add function
    bool execute(string answer) override;
};

//Generate Schedules File class for main menu
class GenerateSchedFile : public ICommand {
public:
    //run add function
    bool execute(string answer) override;
};


#endif
