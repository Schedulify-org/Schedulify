#include "claude_api_integration.h"

struct APIResponse {
    std::string data;
    long response_code;
    bool success;

    APIResponse() : response_code(0), success(false) {}
};

static size_t WriteCallback(void* contents, size_t size, size_t nmemb, APIResponse* response) {
    size_t totalSize = size * nmemb;
    response->data.append((char*)contents, totalSize);
    return totalSize;
}

ClaudeAPIClient::ClaudeAPIClient() {
    curl_global_init(CURL_GLOBAL_DEFAULT);
    Logger::get().logInfo("Claude API client initialized with CURL");
}

ClaudeAPIClient::~ClaudeAPIClient() {
    curl_global_cleanup();
}

BotQueryResponse ClaudeAPIClient::processScheduleQuery(const BotQueryRequest& request) {
    BotQueryResponse response;

    const char* apiKey = getenv("ANTHROPIC_API_KEY");
    if (!apiKey || strlen(apiKey) == 0) {
        Logger::get().logError("ANTHROPIC_API_KEY environment variable not set");
        response.hasError = true;
        response.errorMessage = "API key not configured";
        return response;
    }

    // Clean API key (same as your working version)
    std::string cleanApiKey;
    for (char c : std::string(apiKey)) {
        if (c >= 33 && c <= 126) {  // Printable ASCII range excluding space
            cleanApiKey += c;
        }
    }

    try {
        // Create the request payload
        Json::Value requestJson = createRequestPayload(request);
        Json::StreamWriterBuilder builder;
        std::string jsonString = Json::writeString(builder, requestJson);

        CURL* curl = curl_easy_init();
        if (!curl) {
            response.hasError = true;
            response.errorMessage = "Failed to initialize CURL";
            return response;
        }

        APIResponse apiResponse;

        // CRITICAL FIX 1: Use the correct header format from your working version
        struct curl_slist* headers = nullptr;

        // Your working version uses "x-api-key" instead of "Authorization: Bearer"
        headers = curl_slist_append(headers, ("x-api-key: " + cleanApiKey).c_str());
        headers = curl_slist_append(headers, "Content-Type: application/json");
        headers = curl_slist_append(headers, "anthropic-version: 2023-06-01");  // lowercase 'v'

        // Set CURL options (same as working version)
        curl_easy_setopt(curl, CURLOPT_URL, CLAUDE_API_URL.c_str());
        curl_easy_setopt(curl, CURLOPT_POSTFIELDS, jsonString.c_str());
        curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &apiResponse);
        curl_easy_setopt(curl, CURLOPT_TIMEOUT, 60L);
        curl_easy_setopt(curl, CURLOPT_CONNECTTIMEOUT, 30L);
        curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
        curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 1L);
        curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 2L);

        Logger::get().logInfo("Sending request to Claude API...");

        CURLcode res = curl_easy_perform(curl);
        curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &apiResponse.response_code);

        curl_slist_free_all(headers);
        curl_easy_cleanup(curl);

        Logger::get().logInfo("CURL result: " + std::to_string(res));
        Logger::get().logInfo("Response code: " + std::to_string(apiResponse.response_code));

        if (res != CURLE_OK) {
            response.hasError = true;
            response.errorMessage = "Network error: " + std::string(curl_easy_strerror(res));
            return response;
        }

        if (apiResponse.response_code != 200) {
            Logger::get().logError("Claude API returned HTTP " + std::to_string(apiResponse.response_code));
            Logger::get().logError("Response: " + apiResponse.data);
            response.hasError = true;
            response.errorMessage = "Claude API request failed with HTTP " + std::to_string(apiResponse.response_code);
            return response;
        }

        if (apiResponse.data.empty()) {
            Logger::get().logError("Empty response from Claude API");
            response.hasError = true;
            response.errorMessage = "Empty response from Claude API";
            return response;
        }

        // Parse the response (same parsing logic)
        response = parseClaudeResponse(apiResponse.data);
        Logger::get().logInfo("Claude API request completed successfully");

    } catch (const std::exception& e) {
        Logger::get().logError("Exception in Claude API request: " + std::string(e.what()));
        response.hasError = true;
        response.errorMessage = "Request processing error: " + std::string(e.what());
    }

    return response;
}

Json::Value ClaudeAPIClient::createRequestPayload(const BotQueryRequest& request) {
    Json::Value payload;

    // Set model and parameters
    payload["model"] = CLAUDE_MODEL;
    payload["max_tokens"] = 1024;

    // Create system prompt with schedule metadata
    std::string systemPrompt = createSystemPrompt(request.scheduleMetadata);
    payload["system"] = systemPrompt;

    // Create messages array
    Json::Value messages(Json::arrayValue);
    Json::Value userMessage;
    userMessage["role"] = "user";
    userMessage["content"] = request.userMessage;
    messages.append(userMessage);

    payload["messages"] = messages;

    return payload;
}

std::string ClaudeAPIClient::createSystemPrompt(const std::string& scheduleMetadata) {
    std::string prompt = R"(
You are a helpful schedule filtering assistant. Your job is to help users filter their class schedules based on their preferences.

You have access to a schedule database with the following structure:
)" + scheduleMetadata + R"(

IMPORTANT INSTRUCTIONS:
1. When a user asks to filter schedules, you must provide BOTH:
   - A helpful response explaining what you're doing
   - A SQL query to filter the schedules

2. Your response format must be EXACTLY:
   RESPONSE: [Your helpful message to the user]
   SQL: [Your SQL query]
   PARAMETERS: [Comma-separated list of parameter values, or NONE if no parameters]

3. SQL Query Rules:
   - Always SELECT schedule_index FROM schedule WHERE [conditions]
   - Use ? for parameter binding (never put values directly in SQL)
   - Only use the columns mentioned in the metadata
   - Keep queries simple and safe
   - Only SELECT statements allowed (no INSERT, UPDATE, DELETE)

4. Examples:
   User: "Show me schedules with no gaps"
   RESPONSE: I'll find all schedules that have zero gaps between classes.
   SQL: SELECT schedule_index FROM schedule WHERE amount_gaps = ?
   PARAMETERS: 0

   User: "Find schedules that start after 9 AM"
   RESPONSE: I'll find schedules where the average start time is after 9:00 AM.
   SQL: SELECT schedule_index FROM schedule WHERE avg_start > ?
   PARAMETERS: 540

   User: "What's the best schedule?"
   RESPONSE: I can help you find schedules based on specific criteria like fewer gaps, preferred start times, or number of study days. What matters most to you?
   SQL: NONE
   PARAMETERS: NONE

5. Time conversion:
   - 8:00 AM = 480 minutes from midnight
   - 9:00 AM = 540 minutes from midnight
   - 5:00 PM = 1020 minutes from midnight
   - 6:00 PM = 1080 minutes from midnight

6. If the user asks for general information (not filtering), respond helpfully but set SQL to NONE.

Be conversational and helpful while being precise with your SQL queries.
)";

    return prompt;
}

BotQueryResponse ClaudeAPIClient::parseClaudeResponse(const std::string& responseData) {
    BotQueryResponse botResponse;

    Logger::get().logInfo("=== PARSING CLAUDE RESPONSE ===");
    Logger::get().logInfo("Response data length: " + std::to_string(responseData.length()));

    if (responseData.empty()) {
        Logger::get().logError("Empty response data provided to parser");
        botResponse.hasError = true;
        botResponse.errorMessage = "Empty response from Claude API";
        return botResponse;
    }

    try {
        // Parse JSON response
        Json::Reader reader;
        Json::Value root;

        Logger::get().logInfo("Attempting to parse JSON...");

        if (!reader.parse(responseData, root)) {
            Logger::get().logError("Failed to parse Claude API JSON response");
            Logger::get().logError("JSON parse errors: " + reader.getFormattedErrorMessages());
            Logger::get().logError("Raw response: " + responseData.substr(0, 1000)); // Log first 1000 chars
            botResponse.hasError = true;
            botResponse.errorMessage = "Invalid JSON response from Claude API";
            return botResponse;
        }

        Logger::get().logInfo("JSON parsed successfully");

        // Log the structure of the response for debugging
        Json::StreamWriterBuilder builder;
        builder["indentation"] = "  ";
        std::string prettyJson = Json::writeString(builder, root);
        Logger::get().logInfo("Parsed JSON structure: " + prettyJson.substr(0, 1000)); // First 1000 chars

        // Check for error in response
        if (root.isMember("error")) {
            Logger::get().logError("Error found in Claude response");
            Json::Value error = root["error"];
            std::string errorMessage = "Unknown error";
            std::string errorType = "unknown";

            if (error.isMember("message")) {
                errorMessage = error["message"].asString();
            }
            if (error.isMember("type")) {
                errorType = error["type"].asString();
            }

            Logger::get().logError("Error type: " + errorType + ", message: " + errorMessage);
            botResponse.hasError = true;
            botResponse.errorMessage = errorMessage;
            return botResponse;
        }

        // Extract content
        if (!root.isMember("content")) {
            Logger::get().logError("No 'content' field in Claude API response");
            Logger::get().logError("Available fields: ");
            for (const auto& key : root.getMemberNames()) {
                Logger::get().logError("  - " + key);
            }
            botResponse.hasError = true;
            botResponse.errorMessage = "Invalid response format from Claude API - missing content";
            return botResponse;
        }

        Json::Value content = root["content"];
        if (!content.isArray()) {
            Logger::get().logError("Content field is not an array");
            botResponse.hasError = true;
            botResponse.errorMessage = "Invalid response format from Claude API - content not array";
            return botResponse;
        }

        if (content.empty()) {
            Logger::get().logError("Content array is empty");
            botResponse.hasError = true;
            botResponse.errorMessage = "Empty content in Claude API response";
            return botResponse;
        }

        Json::Value firstContent = content[0];
        if (!firstContent.isMember("text")) {
            Logger::get().logError("No text content in first content item");
            Logger::get().logError("First content item fields:");
            for (const auto& key : firstContent.getMemberNames()) {
                Logger::get().logError("  - " + key);
            }
            botResponse.hasError = true;
            botResponse.errorMessage = "Invalid response format from Claude API - no text in content";
            return botResponse;
        }

        std::string contentText = firstContent["text"].asString();
        Logger::get().logInfo("Extracted content text (length: " + std::to_string(contentText.length()) + ")");
        Logger::get().logInfo("Content preview: " + contentText.substr(0, 300) + "...");

        if (contentText.empty()) {
            Logger::get().logError("Text content is empty");
            botResponse.hasError = true;
            botResponse.errorMessage = "Empty text content from Claude API";
            return botResponse;
        }

        // Parse the structured response
        std::string sqlQuery;
        std::vector<std::string> parameters;

        Logger::get().logInfo("Attempting to extract SQL query...");
        if (extractSQLQuery(contentText, sqlQuery, parameters)) {
            Logger::get().logInfo("SQL query extracted successfully");
            Logger::get().logInfo("SQL: " + sqlQuery);
            Logger::get().logInfo("Parameters count: " + std::to_string(parameters.size()));
            // This is a filter request
            botResponse.isFilterQuery = true;
            botResponse.sqlQuery = sqlQuery;
            botResponse.queryParameters = parameters;
        } else {
            Logger::get().logInfo("No SQL query found - treating as general response");
            // This is a general response
            botResponse.isFilterQuery = false;
        }

        // Extract the user message part
        Logger::get().logInfo("Extracting user message...");
        std::regex responseRegex(R"(RESPONSE:\s*([\s\S]+?)(?:\nSQL:|$))", std::regex_constants::icase);
        std::smatch responseMatch;

        if (std::regex_search(contentText, responseMatch, responseRegex)) {
            botResponse.userMessage = responseMatch[1].str();
            // Trim whitespace
            botResponse.userMessage.erase(0, botResponse.userMessage.find_first_not_of(" \t\n\r"));
            botResponse.userMessage.erase(botResponse.userMessage.find_last_not_of(" \t\n\r") + 1);
            Logger::get().logInfo("Extracted structured response message: " + botResponse.userMessage);
        } else {
            Logger::get().logInfo("No structured response format found, using entire content");
            // Fallback: use the entire content if no structured format
            botResponse.userMessage = contentText;
        }

        if (botResponse.userMessage.empty()) {
            Logger::get().logError("Extracted user message is empty");
            botResponse.hasError = true;
            botResponse.errorMessage = "Empty message extracted from Claude response";
            return botResponse;
        }

        Logger::get().logInfo("=== PARSING COMPLETED SUCCESSFULLY ===");
        Logger::get().logInfo("Final response - isFilter: " + std::to_string(botResponse.isFilterQuery) +
                              ", message length: " + std::to_string(botResponse.userMessage.length()));

    } catch (const std::exception& e) {
        Logger::get().logError("Exception parsing Claude response: " + std::string(e.what()));
        botResponse.hasError = true;
        botResponse.errorMessage = "Failed to parse Claude response: " + std::string(e.what());
    }

    return botResponse;
}

bool ClaudeAPIClient::extractSQLQuery(const std::string& content, std::string& sqlQuery, std::vector<std::string>& parameters) {
    // Extract SQL query
    std::regex sqlRegex(R"(SQL:\s*(.+?)(?:\nPARAMETERS:|$))", std::regex_constants::icase);
    std::smatch sqlMatch;

    if (!std::regex_search(content, sqlMatch, sqlRegex)) {
        return false;
    }

    std::string rawSql = sqlMatch[1].str();
    // Trim whitespace
    rawSql.erase(0, rawSql.find_first_not_of(" \t\n\r"));
    rawSql.erase(rawSql.find_last_not_of(" \t\n\r") + 1);

    // Check if SQL is "NONE"
    if (rawSql == "NONE" || rawSql == "none") {
        return false;
    }

    sqlQuery = rawSql;

    // Extract parameters
    std::regex paramRegex(R"(PARAMETERS:\s*(.+?)(?:\n|$))", std::regex_constants::icase);
    std::smatch paramMatch;

    parameters.clear();

    if (std::regex_search(content, paramMatch, paramRegex)) {
        std::string paramString = paramMatch[1].str();
        // Trim whitespace
        paramString.erase(0, paramString.find_first_not_of(" \t\n\r"));
        paramString.erase(paramString.find_last_not_of(" \t\n\r") + 1);

        if (paramString != "NONE" && paramString != "none" && !paramString.empty()) {
            // Split by comma and clean up
            std::stringstream ss(paramString);
            std::string param;
            while (std::getline(ss, param, ',')) {
                // Trim whitespace from each parameter
                param.erase(0, param.find_first_not_of(" \t\n\r"));
                param.erase(param.find_last_not_of(" \t\n\r") + 1);
                if (!param.empty()) {
                    parameters.push_back(param);
                }
            }
        }
    }

    Logger::get().logInfo("Extracted SQL: " + sqlQuery);
    Logger::get().logInfo("Extracted parameters: " + std::to_string(parameters.size()));

    return true;
}

void ClaudeAPIClient::validateApiKeyEnvironment() {
    const char* apiKey = getenv("ANTHROPIC_API_KEY");

    Logger::get().logInfo("=== API KEY ENVIRONMENT VALIDATION ===");

    if (!apiKey) {
        Logger::get().logError("ANTHROPIC_API_KEY environment variable is not set");
        return;
    }

    std::string rawKey = std::string(apiKey);
    Logger::get().logInfo("Raw API key length: " + std::to_string(rawKey.length()));

    // Check for problematic characters
    bool hasProblems = false;
    for (size_t i = 0; i < rawKey.length(); i++) {
        char c = rawKey[i];
        if (c == '\n') {
            Logger::get().logWarning("API key contains newline at position " + std::to_string(i));
            hasProblems = true;
        }
        if (c == '\r') {
            Logger::get().logWarning("API key contains carriage return at position " + std::to_string(i));
            hasProblems = true;
        }
        if (c == '\t') {
            Logger::get().logWarning("API key contains tab at position " + std::to_string(i));
            hasProblems = true;
        }
        if (c == ' ') {
            Logger::get().logWarning("API key contains space at position " + std::to_string(i));
            hasProblems = true;
        }
        if (c < 32 || c > 126) {
            Logger::get().logWarning("API key contains non-printable character (ASCII " +
                                     std::to_string((int)c) + ") at position " + std::to_string(i));
            hasProblems = true;
        }
    }

    if (!hasProblems) {
        Logger::get().logInfo("API key format appears clean");
    }

    // Show first and last few characters for verification
    if (rawKey.length() > 20) {
        Logger::get().logInfo("API key starts with: '" + rawKey.substr(0, 15) + "'");
        Logger::get().logInfo("API key ends with: '" + rawKey.substr(rawKey.length() - 10) + "'");
    }
}