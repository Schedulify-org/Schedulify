#ifndef CLAUDE_API_INTEGRATION_H
#define CLAUDE_API_INTEGRATION_H

// Include system headers first, before any C++ standard library headers
#include <curl/curl.h>
#include <json/json.h>
#include <cstdlib>  // For getenv

// Then include our project headers
#include "model_interfaces.h"
#include "logger.h"

// Finally include C++ standard library headers
#include <string>
#include <vector>
#include <sstream>
#include <regex>

class ClaudeAPIClient {
public:
    ClaudeAPIClient();
    ~ClaudeAPIClient();

    // Main API method
    BotQueryResponse processScheduleQuery(const BotQueryRequest& request);

private:
    // API interaction methods
    Json::Value createRequestPayload(const BotQueryRequest& request);
    static std::string createSystemPrompt(const std::string& scheduleMetadata);

    static BotQueryResponse parseClaudeResponse(const std::string& responseData);
    static bool extractSQLQuery(const std::string& content, std::string& sqlQuery, std::vector<std::string>& parameters);
    static void validateApiKeyEnvironment();

    const std::string CLAUDE_API_URL = "https://api.anthropic.com/v1/messages";
    const std::string CLAUDE_MODEL = "claude-3-5-sonnet-20241022";
};

#endif // CLAUDE_API_INTEGRATION_H