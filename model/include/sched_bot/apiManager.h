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

enum class Models {
    OPENAI,
    GEMINI
};

string sendToModel(const string& prompt, Models model);

string sendToOpenAI(const string& prompt, const string& apiKey);

string sendToGemini(const string& prompt, const string& apiKey);

string getOpenAIResponse(const string& raw_response);

string getGeminiResponse(const std::string& raw_response);

static size_t WriteCallback(void* contents, size_t size, size_t nmemb, void* userp);

string getApiKey(Models model);

#endif //REST_API_MANAGER_H
