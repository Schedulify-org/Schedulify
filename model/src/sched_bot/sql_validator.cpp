#include "sql_validator.h"

SQLValidator::ValidationResult SQLValidator::validateScheduleQuery(const std::string& sqlQuery) {
    ValidationResult result;

    if (sqlQuery.empty()) {
        result.isValid = false;
        result.errorMessage = "SQL query cannot be empty";
        return result;
    }

    std::string normalizedQuery = normalizeQuery(sqlQuery);
    Logger::get().logInfo("Validating SQL query: " + normalizedQuery);

    // 1. Must be SELECT only
    if (!isSelectOnlyQuery(normalizedQuery)) {
        result.isValid = false;
        result.errorMessage = "Only SELECT queries are allowed";
        return result;
    }

    // 2. Check for forbidden keywords
    if (containsForbiddenKeywords(normalizedQuery)) {
        result.isValid = false;
        result.errorMessage = "Query contains forbidden keywords";
        return result;
    }

    // 3. Must use whitelisted tables only
    if (!usesWhitelistedTablesOnly(normalizedQuery)) {
        result.isValid = false;
        result.errorMessage = "Query uses non-whitelisted tables";
        return result;
    }

    // 4. Must use whitelisted columns only
    if (!usesWhitelistedColumnsOnly(normalizedQuery)) {
        result.isValid = false;
        result.errorMessage = "Query uses non-whitelisted columns";
        return result;
    }

    // 5. Must select schedule_index
    if (!requiresScheduleIndex(normalizedQuery)) {
        result.isValid = false;
        result.errorMessage = "Query must SELECT schedule_index column";
        return result;
    }

    // 6. Parameter count validation
    int paramCount = countParameters(normalizedQuery);
    if (paramCount > 10) { // Reasonable limit
        result.warnings.push_back("Query has many parameters (" + std::to_string(paramCount) + ")");
    }

    Logger::get().logInfo("SQL query validation passed");
    return result;
}

bool SQLValidator::containsForbiddenKeywords(const std::string& query) {
    std::vector<std::string> forbidden = getForbiddenKeywords();
    QString qQuery = QString::fromStdString(query).toLower();

    for (const std::string& keyword : forbidden) {
        QString qKeyword = QString::fromStdString(keyword);

        // Use word boundaries to avoid false positives
        QRegularExpression regex("\\b" + qKeyword + "\\b");
        if (regex.match(qQuery).hasMatch()) {
            Logger::get().logWarning("Found forbidden keyword: " + keyword);
            return true;
        }
    }

    return false;
}

bool SQLValidator::isSelectOnlyQuery(const std::string& query) {
    QString qQuery = QString::fromStdString(query).trimmed().toLower();

    // Must start with SELECT
    if (!qQuery.startsWith("select")) {
        return false;
    }

    // Should not contain other statement keywords
    std::vector<std::string> statementKeywords = {
            "insert", "update", "delete", "create", "drop", "alter", "truncate"
    };

    for (const std::string& keyword : statementKeywords) {
        QRegularExpression regex("\\b" + QString::fromStdString(keyword) + "\\b");
        if (regex.match(qQuery).hasMatch()) {
            return false;
        }
    }

    return true;
}

bool SQLValidator::usesWhitelistedTablesOnly(const std::string& query) {
    std::vector<std::string> tables = extractTableNames(query);
    std::vector<std::string> whitelist = getWhitelistedTables();

    for (const std::string& table : tables) {
        if (std::find(whitelist.begin(), whitelist.end(), table) == whitelist.end()) {
            Logger::get().logWarning("Non-whitelisted table found: " + table);
            return false;
        }
    }

    return true;
}

bool SQLValidator::usesWhitelistedColumnsOnly(const std::string& query) {
    std::vector<std::string> columns = extractColumnNames(query);
    std::vector<std::string> whitelist = getWhitelistedColumns();

    for (const std::string& column : columns) {
        if (column == "*") {
            Logger::get().logWarning("Wildcard (*) not allowed in SELECT");
            return false;
        }

        if (std::find(whitelist.begin(), whitelist.end(), column) == whitelist.end()) {
            Logger::get().logWarning("Non-whitelisted column found: " + column);
            return false;
        }
    }

    return true;
}

bool SQLValidator::requiresScheduleIndex(const std::string& query) {
    QString qQuery = QString::fromStdString(query).toLower();

    // Check if schedule_index is in the SELECT clause
    QRegularExpression selectRegex(R"(select\s+(.+?)\s+from)", QRegularExpression::DotMatchesEverythingOption);
    QRegularExpressionMatch match = selectRegex.match(qQuery);

    if (!match.hasMatch()) {
        return false;
    }

    QString selectClause = match.captured(1);
    return selectClause.contains("schedule_index");
}

int SQLValidator::countParameters(const std::string& query) {
    return std::count(query.begin(), query.end(), '?');
}

std::vector<std::string> SQLValidator::extractTableNames(const std::string& query) {
    std::vector<std::string> tables;
    QString qQuery = QString::fromStdString(query).toLower();

    // Simple extraction using FROM clause
    QRegularExpression fromRegex(R"(from\s+(\w+))");
    QRegularExpressionMatchIterator it = fromRegex.globalMatch(qQuery);

    while (it.hasNext()) {
        QRegularExpressionMatch match = it.next();
        std::string tableName = match.captured(1).toStdString();
        if (std::find(tables.begin(), tables.end(), tableName) == tables.end()) {
            tables.push_back(tableName);
        }
    }

    // Also check JOIN clauses
    QRegularExpression joinRegex(R"(join\s+(\w+))");
    it = joinRegex.globalMatch(qQuery);

    while (it.hasNext()) {
        QRegularExpressionMatch match = it.next();
        std::string tableName = match.captured(1).toStdString();
        if (std::find(tables.begin(), tables.end(), tableName) == tables.end()) {
            tables.push_back(tableName);
        }
    }

    return tables;
}

std::vector<std::string> SQLValidator::extractColumnNames(const std::string& query) {
    std::vector<std::string> columns;
    QString qQuery = QString::fromStdString(query).toLower();

    // Extract from SELECT clause
    QRegularExpression selectRegex(R"(select\s+(.+?)\s+from)", QRegularExpression::DotMatchesEverythingOption);
    QRegularExpressionMatch match = selectRegex.match(qQuery);

    if (match.hasMatch()) {
        QString selectClause = match.captured(1);
        QStringList columnList = selectClause.split(',');

        for (QString column : columnList) {
            column = column.trimmed();
            // Remove table prefixes (e.g., "table.column" -> "column")
            if (column.contains('.')) {
                column = column.split('.').last();
            }
            // Remove alias (e.g., "column AS alias" -> "column")
            if (column.contains(" as ")) {
                column = column.split(" as ").first().trimmed();
            }

            std::string columnName = column.toStdString();
            if (!columnName.empty() && std::find(columns.begin(), columns.end(), columnName) == columns.end()) {
                columns.push_back(columnName);
            }
        }
    }

    // Extract from WHERE clause
    QRegularExpression whereRegex(R"(where\s+(.+?)(?:\s+order|\s+group|\s+limit|$))", QRegularExpression::DotMatchesEverythingOption);
    match = whereRegex.match(qQuery);

    if (match.hasMatch()) {
        QString whereClause = match.captured(1);
        // Extract column names from conditions (simplified)
        QRegularExpression columnRegex(R"(\b(\w+)\s*[=<>!])");
        QRegularExpressionMatchIterator it = columnRegex.globalMatch(whereClause);

        while (it.hasNext()) {
            QRegularExpressionMatch columnMatch = it.next();
            std::string columnName = columnMatch.captured(1).toStdString();
            if (std::find(columns.begin(), columns.end(), columnName) == columns.end()) {
                columns.push_back(columnName);
            }
        }
    }

    return columns;
}

std::vector<std::string> SQLValidator::getForbiddenKeywords() {
    return {
            "insert", "update", "delete", "drop", "create", "alter",
            "truncate", "grant", "revoke", "exec", "execute",
            "declare", "cast", "convert", "union", "into",
            "merge", "replace", "call", "do", "handler",
            "load", "rename", "optimize", "repair", "analyze",
            "check", "checksum", "restore", "backup",
            "show", "describe", "explain" // Info commands that might leak schema
    };
}

std::vector<std::string> SQLValidator::getWhitelistedTables() {
    return {
            "schedule", "schedule_set"
    };
}

std::vector<std::string> SQLValidator::getWhitelistedColumns() {
    return {
            "schedule_index", "amount_days", "amount_gaps", "gaps_time",
            "avg_start", "avg_end", "id", "schedule_set_id", "created_at",
            "set_name", "source_file_ids_json", "schedule_count"
    };
}

std::string SQLValidator::sanitizeQuery(const std::string& query) {
    std::string sanitized = query;

    // Remove comments
    size_t pos = 0;
    while ((pos = sanitized.find("--", pos)) != std::string::npos) {
        size_t endPos = sanitized.find('\n', pos);
        if (endPos == std::string::npos) {
            sanitized.erase(pos);
            break;
        } else {
            sanitized.erase(pos, endPos - pos);
        }
    }

    // Remove block comments
    pos = 0;
    while ((pos = sanitized.find("/*", pos)) != std::string::npos) {
        size_t endPos = sanitized.find("*/", pos);
        if (endPos != std::string::npos) {
            sanitized.erase(pos, endPos - pos + 2);
        } else {
            sanitized.erase(pos);
            break;
        }
    }

    return sanitized;
}

std::string SQLValidator::normalizeQuery(const std::string& query) {
    std::string normalized = sanitizeQuery(query);

    // Convert to lowercase for analysis
    std::transform(normalized.begin(), normalized.end(), normalized.begin(), ::tolower);

    // Replace multiple whitespace with single space
    QString qNormalized = QString::fromStdString(normalized);
    qNormalized = qNormalized.simplified();

    return qNormalized.toStdString();
}

bool SQLValidator::matchesPattern(const QString& query, const QString& pattern) {
    QRegularExpression regex(pattern, QRegularExpression::CaseInsensitiveOption);
    return regex.match(query).hasMatch();
}

std::vector<std::string> SQLValidator::tokenizeQuery(const std::string& query) {
    std::vector<std::string> tokens;
    QString qQuery = QString::fromStdString(query);

    // Simple tokenization by whitespace and common SQL delimiters
    QRegularExpression tokenRegex(R"([\s,()=<>!]+)");
    QStringList tokenList = qQuery.split(tokenRegex, Qt::SkipEmptyParts);

    for (const QString& token : tokenList) {
        if (!token.trimmed().isEmpty()) {
            tokens.push_back(token.trimmed().toStdString());
        }
    }

    return tokens;
}

bool SQLValidator::isValidIdentifier(const std::string& identifier) {
    if (identifier.empty()) {
        return false;
    }

    // Must start with letter or underscore
    if (!std::isalpha(identifier[0]) && identifier[0] != '_') {
        return false;
    }

    // Rest must be alphanumeric or underscore
    for (size_t i = 1; i < identifier.length(); ++i) {
        if (!std::isalnum(identifier[i]) && identifier[i] != '_') {
            return false;
        }
    }

    return true;
}