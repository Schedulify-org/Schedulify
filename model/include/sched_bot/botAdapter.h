#ifndef SCHED_BOT_H
#define SCHED_BOT_H

#include "apiManager.h"

#include <iostream>
#include <string>
#include <vector>
#include <sstream>
#include <cstdlib>
#include <json/json.h>
#include <cctype>

using namespace std;

vector<string> askModel(const string& question, const string& data);

int extractScheduleIndex(const string& text);

#endif //SCHED_BOT_H
