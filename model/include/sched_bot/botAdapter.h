#ifndef SCHED_BOT_H
#define SCHED_BOT_H

#include "apiManager.h"

#include <iostream>
#include <string>
#include <vector>
#include <sstream>
#include <cstdlib>
#include <json/json.h>

using namespace std;

enum class QuestionType {
    FIND_ME,
    TELL_ME
};

vector<string> askModel(const string& question, const string& data, QuestionType type);

vector<string> getFormatedResponse(const string& raw_response);

#endif //SCHED_BOT_H
