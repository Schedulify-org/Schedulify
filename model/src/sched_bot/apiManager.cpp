#include "apiManager.h"

static size_t WriteCallback(void* contents, size_t size, size_t nmemb, void* userp) {
    ((string*)userp)->append((char*)contents, size * nmemb);
    return size * nmemb;
}

string sendToModel(const string& prompt, Models model) {
    string result;
    string apiKey = getApiKey(model);
    switch (model) {
        case Models::OPENAI:
            result = sendToOpenAI(prompt, apiKey);
            return getOpenAIResponse(result);

        case Models::GEMINI:
            result = sendToGemini(prompt, apiKey);
            return getGeminiResponse(result);
    }
}

string sendToOpenAI(const string& prompt, const string& apiKey) {
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

std::string sendToGemini(const std::string& prompt, const string& apiKey) {
    CURL* curl = curl_easy_init();
    if (!curl) return "Failed to initialize curl";

    std::string readBuffer;

    Json::Value root;
    Json::Value part(Json::objectValue);
    part["text"] = prompt;

    Json::Value parts(Json::arrayValue);
    parts.append(part);

    Json::Value content(Json::objectValue);
    content["parts"] = parts;

    Json::Value contents(Json::arrayValue);
    contents.append(content);

    root["contents"] = contents;

    Json::StreamWriterBuilder writer;
    std::string jsonData = Json::writeString(writer, root);

    std::string url = "https://generativelanguage.googleapis.com/v1beta/models/gemini-2.0-flash:generateContent?key=" + apiKey;

    struct curl_slist* headers = nullptr;
    headers = curl_slist_append(headers, "Content-Type: application/json");

    curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
    curl_easy_setopt(curl, CURLOPT_POSTFIELDS, jsonData.c_str());
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &readBuffer);

    CURLcode res = curl_easy_perform(curl);
    if (res != CURLE_OK) {
        std::cerr << "curl_easy_perform() failed: " << curl_easy_strerror(res) << "\n";
    }

    curl_easy_cleanup(curl);
    curl_slist_free_all(headers);

    return readBuffer;
}

string getOpenAIResponse(const string& raw_response) {
    string replyText;
    Json::CharReaderBuilder reader;
    Json::Value root;
    istringstream s(raw_response);
    string errs;

    if (Json::parseFromStream(reader, s, &root, &errs)) {
        try {
            replyText = root["choices"][0]["message"]["content"].asString();
        } catch (...) {
            replyText = "[Error extracting response]";
        }
    } else {
        replyText = "[Invalid JSON from model]";
    }

    return replyText;
}

string getGeminiResponse(const std::string& raw_response) {
    std::string replyText;
    Json::CharReaderBuilder reader;
    Json::Value root;
    std::istringstream s(raw_response);
    std::string errs;

    if (Json::parseFromStream(reader, s, &root, &errs)) {
        try {
            // --- MODIFIED for Gemini API response structure ---
            // The text content is usually found in:
            // root["candidates"][0]["content"]["parts"][0]["text"]
            if (root.isMember("candidates") && root["candidates"].isArray() && root["candidates"].size() > 0) {
                const Json::Value& candidate = root["candidates"][0];
                if (candidate.isMember("content") && candidate["content"].isMember("parts") && candidate["content"]["parts"].isArray() && candidate["content"]["parts"].size() > 0) {
                    replyText = candidate["content"]["parts"][0]["text"].asString();
                } else {
                    replyText = "[Error: Content or parts not found in Gemini response]";
                }
            } else {
                replyText = "[Error: Candidates not found in Gemini response]";
            }
        } catch (const std::exception& e) {
            std::cerr << "Exception extracting Gemini response: " << e.what() << std::endl;
            replyText = "[Error extracting response]";
        } catch (...) {
            std::cerr << "Unknown exception extracting Gemini response." << std::endl;
            replyText = "[Error extracting response]";
        }
    } else {
        std::cerr << "JSON parsing error: " << errs << std::endl;
        replyText = "[Invalid JSON from model]";
    }

    return replyText;
}

string getApiKey(Models model) {
    const char *apiType;
    switch (model) {
        case Models::OPENAI:
            apiType = "OPENAI_API_KEY";
            break;
        case Models::GEMINI:
            apiType = "GEMINI_API_KEY";
            break;
    }
    const char* key = getenv(apiType);
    if (key == nullptr) {
        throw runtime_error("API key not found in environment variables");
    }
    return {key};
}
