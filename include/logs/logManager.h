#ifndef SCHED_LOG_MANAGER_H
#define SCHED_LOG_MANAGER_H

#include <string>
#include <sstream>
#include <iomanip>
#include <fstream>
#include <iostream>

using std::string;
using std::ostringstream;
using std::put_time;
using std::ofstream;
using std::endl;
using std::ios;

string getTimestampedLogPath();
string getTimeStamp();
void createLogFile(const string& path);
void appendToLogFile(const string& logPath, const string& message);

#endif //SCHED_LOG_MANAGER_H
