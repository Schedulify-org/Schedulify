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

    // Try to get API key from environment variable
    m_apiKey = getApiKeyFromEnvironment();

    if (m_apiKey.empty()) {
        Logger::get().logWarning("ANTHROPIC_API_KEY environment variable not set");
    } else {
        Logger::get().logInfo("Claude API key loaded from environment variable");
    }

    Logger::get().logInfo("Claude API client initialized with CURL");
}

ClaudeAPIClient::~ClaudeAPIClient() {
    curl_global_cleanup();
}

BotQueryResponse ClaudeAPIClient::processScheduleQuery(const BotQueryRequest& request) {
    BotQueryResponse response;

    if (m_apiKey.empty()) {
        Logger::get().logError("Claude API key not configured");
        response.hasError = true;
        response.errorMessage = "API key not configured. Please check your settings.";
        return response;
    }

    try {
        Logger::get().logInfo("Processing schedule query with CURL: " + request.userMessage);

        // Create the request payload
        Json::Value requestJson = createRequestPayload(request);
        Json::StreamWriterBuilder builder;
        std::string jsonString = Json::writeString(builder, requestJson);

        // Initialize CURL
        CURL* curl = curl_easy_init();
        if (!curl) {
            response.hasError = true;
            response.errorMessage = "Failed to initialize CURL";
            return response;
        }

        APIResponse apiResponse;

        // Set CURL options
        curl_easy_setopt(curl, CURLOPT_URL, CLAUDE_API_URL.c_str());
        curl_easy_setopt(curl, CURLOPT_POSTFIELDS, jsonString.c_str());
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &apiResponse);
        curl_easy_setopt(curl, CURLOPT_TIMEOUT, 30L); // 30 second timeout

        // Set headers
        struct curl_slist* headers = nullptr;
        std::string authHeader = "Authorization: Bearer " + m_apiKey;
        std::string contentType = "Content-Type: application/json";
        std::string anthropicVersion = "Anthropic-Version: 2023-06-01";
        std::string userAgent = "User-Agent: Schedulify/1.0";

        headers = curl_slist_append(headers, authHeader.c_str());
        headers = curl_slist_append(headers, contentType.c_str());
        headers = curl_slist_append(headers, anthropicVersion.c_str());
        headers = curl_slist_append(headers, userAgent.c_str());
        curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);

        // Perform the request
        CURLcode res = curl_easy_perform(curl);
        curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &apiResponse.response_code);

        // Cleanup
        curl_slist_free_all(headers);
        curl_easy_cleanup(curl);

        if (res != CURLE_OK) {
            Logger::get().logError("CURL request failed: " + std::string(curl_easy_strerror(res)));
            response.hasError = true;
            response.errorMessage = "Network request failed. Please check your connection.";
            return response;
        }

        if (apiResponse.response_code != 200) {
            Logger::get().logError("Claude API returned HTTP " + std::to_string(apiResponse.response_code));
            Logger::get().logError("Response: " + apiResponse.data);
            response.hasError = true;
            response.errorMessage = "Claude API request failed. Please try again.";
            return response;
        }

        // Parse the response
        response = parseClaudeResponse(apiResponse.data);
        Logger::get().logInfo("Claude API request completed successfully");

    } catch (const std::exception& e) {
        Logger::get().logError("Exception in Claude API request: " + std::string(e.what()));
        response.hasError = true;
        response.errorMessage = "An error occurred while processing your request.";
    }

    return response;
}

void ClaudeAPIClient::setApiKey(const std::string& apiKey) {
    m_apiKey = apiKey;
    Logger::get().logInfo("Claude API key configured manually");
}

bool ClaudeAPIClient::isApiKeyConfigured() const {
    return !m_apiKey.empty() && m_apiKey.length() > 10; // Basic validation
}

std::string ClaudeAPIClient::getApiKeyFromEnvironment() const {
    const char* envApiKey = std::getenv("ANTHROPIC_API_KEY");
    if (envApiKey != nullptr) {
        return std::string(envApiKey);
    }
    return "";
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

    try {
        // Parse JSON response
        Json::Reader reader;
        Json::Value root;

        if (!reader.parse(responseData, root)) {
            Logger::get().logError("Failed to parse Claude API JSON response");
            botResponse.hasError = true;
            botResponse.errorMessage = "Invalid response from Claude API";
            return botResponse;
        }

        // Extract content
        if (!root.isMember("content") || !root["content"].isArray() || root["content"].empty()) {
            Logger::get().logError("No content in Claude API response");
            botResponse.hasError = true;
            botResponse.errorMessage = "Empty response from Claude API";
            return botResponse;
        }

        Json::Value firstContent = root["content"][0];
        if (!firstContent.isMember("text")) {
            Logger::get().logError("No text content in Claude response");
            botResponse.hasError = true;
            botResponse.errorMessage = "Invalid response format from Claude API";
            return botResponse;
        }

        std::string content = firstContent["text"].asString();
        Logger::get().logInfo("Claude response content: " + content);

        // Parse the structured response
        std::string sqlQuery;
        std::vector<std::string> parameters;

        if (extractSQLQuery(content, sqlQuery, parameters)) {
            // This is a filter request
            botResponse.isFilterQuery = true;
            botResponse.sqlQuery = sqlQuery;
            botResponse.queryParameters = parameters;
        } else {
            // This is a general response
            botResponse.isFilterQuery = false;
        }

        // Extract the user message part
        std::regex responseRegex(R"(RESPONSE:\s*(.+?)(?:\nSQL:|$))", std::regex_constants::icase);
        std::smatch responseMatch;

        if (std::regex_search(content, responseMatch, responseRegex)) {
            botResponse.userMessage = responseMatch[1].str();
            // Trim whitespace
            botResponse.userMessage.erase(0, botResponse.userMessage.find_first_not_of(" \t\n\r"));
            botResponse.userMessage.erase(botResponse.userMessage.find_last_not_of(" \t\n\r") + 1);
        } else {
            // Fallback: use the entire content if no structured format
            botResponse.userMessage = content;
        }

        Logger::get().logInfo("Parsed bot response - isFilter: " + std::to_string(botResponse.isFilterQuery) +
                              ", message: " + botResponse.userMessage);

    } catch (const std::exception& e) {
        Logger::get().logError("Exception parsing Claude response: " + std::string(e.what()));
        botResponse.hasError = true;
        botResponse.errorMessage = "Failed to parse Claude response";
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