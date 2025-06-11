#ifndef REST_API_MANAGER_H
#define REST_API_MANAGER_H

#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <curl/curl.h>
#include <sstream>
#include <cstdlib>
#include <json/json.h>

using namespace std;

static size_t WriteCallback(void* contents, size_t size, size_t nmemb, void* userp);

string sendToModel(const string& prompt);

void loadEnvFile();

std::string getApiKey();

#endif //REST_API_MANAGER_H
