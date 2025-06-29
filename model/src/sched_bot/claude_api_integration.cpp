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
You are SchedBot, an expert schedule filtering assistant. Your job is to analyze user requests and generate SQL queries to filter class schedules.

<schedule_data>
)" + scheduleMetadata + R"(
</schedule_data>

<instructions>
When a user asks to filter schedules, you MUST respond in this EXACT format:

RESPONSE: [Your helpful explanation of what you're filtering for]
SQL: [The SQL query to execute]
PARAMETERS: [Comma-separated parameter values, or NONE]

For non-filtering questions, respond normally and set SQL to NONE.
</instructions>

<examples>
<example_1>
User: "Find schedules that start after 9 AM"
RESPONSE: I'll find schedules where classes typically start after 9:00 AM, giving you a more relaxed morning schedule.
SQL: SELECT schedule_index FROM schedule WHERE avg_start > ?
PARAMETERS: 540
</example_1>

<example_2>
User: "Show me schedules with no gaps"
RESPONSE: I'll find schedules with zero gaps between classes, giving you continuous class time.
SQL: SELECT schedule_index FROM schedule WHERE amount_gaps = ?
PARAMETERS: 0
</example_2>

<example_3>
User: "I want maximum 4 study days"
RESPONSE: I'll find schedules that spread your classes across 4 days or fewer, giving you more free days.
SQL: SELECT schedule_index FROM schedule WHERE amount_days <= ?
PARAMETERS: 4
</example_3>

<example_4>
User: "What's the best schedule?"
RESPONSE: That depends on your preferences! I can help you find schedules based on specific criteria like fewer gaps, preferred start times, number of study days, or ending times. What matters most to you?
SQL: NONE
PARAMETERS: NONE
</example_4>
</examples>

<time_reference>
Time conversion (minutes from midnight):
- 8:00 AM = 480 minutes
- 9:00 AM = 540 minutes
- 10:00 AM = 600 minutes
- 11:00 AM = 660 minutes
- 12:00 PM = 720 minutes
- 1:00 PM = 780 minutes
- 2:00 PM = 840 minutes
- 3:00 PM = 900 minutes
- 4:00 PM = 960 minutes
- 5:00 PM = 1020 minutes
- 6:00 PM = 1080 minutes
</time_reference>

<sql_rules>
- Always SELECT schedule_index FROM schedule WHERE [conditions]
- Use ? for parameters, never hardcode values
- Only use columns: schedule_index, amount_days, amount_gaps, gaps_time, avg_start, avg_end
- Only SELECT statements allowed
</sql_rules>

Remember: You MUST follow the exact response format with RESPONSE:, SQL:, and PARAMETERS: labels.
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

        if (!reader.parse(responseData, root)) {
            Logger::get().logError("Failed to parse Claude API JSON response");
            Logger::get().logError("Raw response (first 500 chars): " + responseData.substr(0, 500));
            botResponse.hasError = true;
            botResponse.errorMessage = "Invalid JSON response from Claude API";
            return botResponse;
        }

        // Check for error in response
        if (root.isMember("error")) {
            Json::Value error = root["error"];
            std::string errorMessage = error.isMember("message") ? error["message"].asString() : "Unknown error";
            Logger::get().logError("Error in Claude response: " + errorMessage);
            botResponse.hasError = true;
            botResponse.errorMessage = errorMessage;
            return botResponse;
        }

        // Extract content
        if (!root.isMember("content") || !root["content"].isArray() || root["content"].empty()) {
            Logger::get().logError("Invalid content structure in Claude response");
            botResponse.hasError = true;
            botResponse.errorMessage = "Invalid response format from Claude API";
            return botResponse;
        }

        Json::Value firstContent = root["content"][0];
        if (!firstContent.isMember("text")) {
            Logger::get().logError("No text content in Claude response");
            botResponse.hasError = true;
            botResponse.errorMessage = "No text content in Claude API response";
            return botResponse;
        }

        std::string contentText = firstContent["text"].asString();
        Logger::get().logInfo("Extracted content text (length: " + std::to_string(contentText.length()) + ")");
        Logger::get().logInfo("Full content text: " + contentText);

        if (contentText.empty()) {
            Logger::get().logError("Text content is empty");
            botResponse.hasError = true;
            botResponse.errorMessage = "Empty text content from Claude API";
            return botResponse;
        }

        // Parse the structured response
        std::string sqlQuery;
        std::vector<std::string> parameters;

        Logger::get().logInfo("Attempting to extract SQL query and parameters...");
        if (extractSQLQuery(contentText, sqlQuery, parameters)) {
            Logger::get().logInfo("✅ SQL EXTRACTION SUCCESSFUL");
            Logger::get().logInfo("SQL Query: " + sqlQuery);
            Logger::get().logInfo("Parameters: " + std::to_string(parameters.size()) + " found");
            for (size_t i = 0; i < parameters.size(); ++i) {
                Logger::get().logInfo("  Parameter " + std::to_string(i) + ": " + parameters[i]);
            }

            botResponse.isFilterQuery = true;
            botResponse.sqlQuery = sqlQuery;
            botResponse.queryParameters = parameters;
        } else {
            Logger::get().logInfo("❌ No SQL query found - treating as general response");
            botResponse.isFilterQuery = false;
        }

        // Extract the user message part - try structured format first using simple string parsing
        auto toLowerCase = [](std::string str) {
            std::transform(str.begin(), str.end(), str.begin(), ::tolower);
            return str;
        };

        auto trim = [](std::string str) {
            str.erase(0, str.find_first_not_of(" \t\n\r"));
            str.erase(str.find_last_not_of(" \t\n\r") + 1);
            return str;
        };

        std::string lowerContent = toLowerCase(contentText);
        size_t responsePos = lowerContent.find("response:");

        if (responsePos != std::string::npos) {
            // Start after "RESPONSE:"
            size_t responseStartPos = responsePos + 9; // Length of "response:"

            // Find the end of response section (look for "SQL:" or end of string)
            size_t responseEndPos = lowerContent.find("sql:", responseStartPos);
            if (responseEndPos == std::string::npos) {
                responseEndPos = contentText.length();
            }

            // Extract response content (use original content, not lowercase)
            std::string responseContent = contentText.substr(responseStartPos, responseEndPos - responseStartPos);
            botResponse.userMessage = trim(responseContent);
            Logger::get().logInfo("Extracted structured response message: " + botResponse.userMessage);
        } else {
            Logger::get().logInfo("No structured RESPONSE: format found, using entire content");
            // Fallback: use the entire content
            botResponse.userMessage = contentText;
        }

        if (botResponse.userMessage.empty()) {
            Logger::get().logError("Extracted user message is empty");
            botResponse.hasError = true;
            botResponse.errorMessage = "Empty message extracted from Claude response";
            return botResponse;
        }

        Logger::get().logInfo("=== PARSING COMPLETED SUCCESSFULLY ===");
        Logger::get().logInfo("Final response summary:");
        Logger::get().logInfo("  - Is filter query: " + std::string(botResponse.isFilterQuery ? "YES" : "NO"));
        Logger::get().logInfo("  - Message length: " + std::to_string(botResponse.userMessage.length()));
        Logger::get().logInfo("  - SQL query: " + (botResponse.isFilterQuery ? botResponse.sqlQuery : "NONE"));

    } catch (const std::exception& e) {
        Logger::get().logError("Exception parsing Claude response: " + std::string(e.what()));
        botResponse.hasError = true;
        botResponse.errorMessage = "Failed to parse Claude response: " + std::string(e.what());
    }

    return botResponse;
}

bool ClaudeAPIClient::extractSQLQuery(const std::string& content, std::string& sqlQuery, std::vector<std::string>& parameters) {
    Logger::get().logInfo("=== SQL EXTRACTION DEBUG ===");
    Logger::get().logInfo("Raw content to parse: " + content);

    // Helper function to convert to lowercase for comparison
    auto toLowerCase = [](std::string str) {
        std::transform(str.begin(), str.end(), str.begin(), ::tolower);
        return str;
    };

    // Helper function to trim whitespace
    auto trim = [](std::string str) {
        str.erase(0, str.find_first_not_of(" \t\n\r"));
        str.erase(str.find_last_not_of(" \t\n\r") + 1);
        return str;
    };

    std::string lowerContent = toLowerCase(content);

    // Find SQL section
    size_t sqlPos = lowerContent.find("sql:");
    if (sqlPos == std::string::npos) {
        Logger::get().logError("No 'SQL:' marker found in content");
        return false;
    }

    Logger::get().logInfo("Found SQL marker at position: " + std::to_string(sqlPos));

    // Start after "SQL:"
    size_t sqlStartPos = sqlPos + 4; // Length of "sql:"

    // Find the end of SQL section (look for "PARAMETERS:" or end of string)
    size_t sqlEndPos = lowerContent.find("parameters:", sqlStartPos);
    if (sqlEndPos == std::string::npos) {
        sqlEndPos = content.length();
    }

    Logger::get().logInfo("SQL section ends at position: " + std::to_string(sqlEndPos));

    // Extract SQL content (use original content, not lowercase)
    std::string rawSql = content.substr(sqlStartPos, sqlEndPos - sqlStartPos);
    Logger::get().logInfo("Raw SQL extracted: '" + rawSql + "'");

    // Trim the SQL
    rawSql = trim(rawSql);
    Logger::get().logInfo("Trimmed SQL: '" + rawSql + "'");

    // Check if SQL is "NONE" or empty
    std::string lowerSql = toLowerCase(rawSql);
    if (lowerSql == "none" || rawSql.empty()) {
        Logger::get().logInfo("SQL is NONE or empty - not a filter query");
        return false;
    }

    sqlQuery = rawSql;
    Logger::get().logInfo("Final SQL query: " + sqlQuery);

    // Find PARAMETERS section
    parameters.clear();

    size_t paramPos = lowerContent.find("parameters:");
    if (paramPos != std::string::npos) {
        Logger::get().logInfo("Found PARAMETERS marker at position: " + std::to_string(paramPos));

        // Start after "PARAMETERS:"
        size_t paramStartPos = paramPos + 11; // Length of "parameters:"

        // Find end of line or end of string
        size_t paramEndPos = content.find('\n', paramStartPos);
        if (paramEndPos == std::string::npos) {
            paramEndPos = content.length();
        }

        // Extract parameters content (use original content, not lowercase)
        std::string rawParams = content.substr(paramStartPos, paramEndPos - paramStartPos);
        Logger::get().logInfo("Raw parameters extracted: '" + rawParams + "'");

        // Trim the parameters
        rawParams = trim(rawParams);
        Logger::get().logInfo("Trimmed parameters: '" + rawParams + "'");

        // Check if parameters is "NONE" or empty
        std::string lowerParams = toLowerCase(rawParams);
        if (lowerParams != "none" && !rawParams.empty()) {
            // Split by comma if multiple parameters
            std::stringstream ss(rawParams);
            std::string param;
            while (std::getline(ss, param, ',')) {
                param = trim(param);
                if (!param.empty()) {
                    parameters.push_back(param);
                    Logger::get().logInfo("Added parameter: '" + param + "'");
                }
            }
        }
    } else {
        Logger::get().logWarning("No PARAMETERS marker found");
    }

    Logger::get().logInfo("Final extraction - SQL: '" + sqlQuery + "', Parameters: " + std::to_string(parameters.size()));

    return true;
}