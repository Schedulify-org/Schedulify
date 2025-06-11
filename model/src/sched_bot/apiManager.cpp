#include "openAI.h"

static size_t WriteCallback(void* contents, size_t size, size_t nmemb, void* userp) {
    ((string*)userp)->append((char*)contents, size * nmemb);
    return size * nmemb;
}

string sendToModel(const string& prompt) {
    loadEnvFile();
    string apiKey = getApiKey();
    if (apiKey.empty()) {
        cerr << "API_KEY not set in environment.\n";
        return "";
    }

    CURL* curl = curl_easy_init();
    if (!curl) return "Failed to initialize curl";

    string readBuffer;

    Json::Value root;
    root["model"] = "gpt-4o";
    root["temperature"] = 0;

    Json::Value msg(Json::objectValue);
    msg["role"] = "user";
    msg["content"] = prompt;

    Json::Value messages(Json::arrayValue);
    messages.append(msg);

    root["messages"] = messages;

    Json::StreamWriterBuilder writer;
    std::string jsonData = Json::writeString(writer, root);

    struct curl_slist* headers = nullptr;
    headers = curl_slist_append(headers, ("Authorization: Bearer " + apiKey).c_str());
    headers = curl_slist_append(headers, "Content-Type: application/json");

    curl_easy_setopt(curl, CURLOPT_URL, "https://api.openai.com/v1/chat/completions");
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
    curl_easy_setopt(curl, CURLOPT_POSTFIELDS, jsonData.c_str());
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &readBuffer);

    CURLcode res = curl_easy_perform(curl);
    if (res != CURLE_OK) {
        cerr << "curl_easy_perform() failed: " << curl_easy_strerror(res) << "\n";
    }

    curl_easy_cleanup(curl);
    curl_slist_free_all(headers);

    return readBuffer;
}

std::string getApiKey() {
    const char* apiKeyPtr = getenv("API_KEY");

    if (apiKeyPtr == nullptr) {
        // API_KEY environment variable is not set
        std::cerr << "Error: API_KEY environment variable not found!" << std::endl;
        return ""; // Return empty string instead of crashing
    }

    return std::string(apiKeyPtr);
}

void loadEnvFile() {
    ifstream file(".env");
    if (!file.is_open()) return;

    std::string line;
    while (std::getline(file, line)) {
        if (line.find("API_KEY=") == 0) {
            std::string value = line.substr(8); // Skip "API_KEY="
            #ifdef _WIN32
            _putenv(("API_KEY=" + value).c_str());
            #else
            setenv("API_KEY", value.c_str(), 1);
            #endif
            break;
        }
    }
}
