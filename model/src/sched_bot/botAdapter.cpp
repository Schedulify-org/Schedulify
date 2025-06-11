#include "botAdapter.h"

std::vector<std::string> askModel(const std::string& question, const std::string& data) {
    std::string explanation =
            "You will receive a list of schedule metadata objects. Each object represents a possible weekly schedule with the following structure:\n"
            "\nSTRUCTURE: ScheduleMeta\n"
            "- index: Unique identifier for the schedule.\n"
            "- amount_days: Number of study days in the week (Sunday to Friday).\n"
            "- amount_gaps: Number of session gaps in the schedule.\n"
            "- gaps_time: Total minutes of gaps.\n"
            "- avg_start: Average daily start time in minutes (e.g. 600 = 10:00 AM).\n"
            "- avg_end: Average daily end time in minutes.\n"
            "- total_minutes: Total minutes spent in all sessions.\n"
            "- total_sessions: Number of sessions.\n"
            "- max_sessions_per_day: Maximum sessions in any one day.\n"
            "- days_with_single_session: Number of days with only one session.\n"
            "- blocked_time_minutes: Minutes of user-defined unavailable time.\n"
            "- has_long_gap: True if any gap is 2 hours or longer.\n"
            "- has_morning_session: True if any session starts before 9:00 AM.\n"
            "- has_late_session: True if any session ends after 6:00 PM.\n"
            "- has_midweek_break: True if there's a free day between study days.\n"
            "- has_back_to_back: True if any sessions have ≤15 min gap.\n"
            "- is_balanced: True if session time is spread evenly.\n"
            "- is_compact: True if total class time is high relative to blocked time.\n"
            "\nARRAY: week (DayMeta)\n"
            "- day: The name of the day (Sunday–Friday).\n"
            "- num_sessions: Sessions that day.\n"
            "- total_minutes: Minutes in class that day.\n"
            "- earliest_start / latest_end: Time in minutes.\n"
            "- longest_gap: Longest time between sessions.\n"
            "\nARRAY: courses (CourseMeta)\n"
            "- raw_id / name: Identifier and title.\n"
            "- lectures / tutorials / labs: Number of sessions.\n"
            "- earliest_start / latest_end: Session time range for this course.\n"
            "- we are in israel, so when i ask you how many free days i have in the schedule, count the free days between "
            "sunday, monday, tuesday, wednesday and thursday.\n"
            "- when i ask for a schedule that doesnt start before a specific time, it means that there are no days with "
            "sessions that starts earlier from the given time.\n"
            "- when i ask for a schedule that doesnt end later a specific time, it means that there are no days with "
            "sessions that ends later from the given time.\n"
            "- when user asks to jump to a schedule by index, write jumping to index {x}.\n";

    std::string prompt =
            explanation +
            "\nHere is a list of schedule metadata entries:\n" + data +
            "\n\nNow, answer the following question based on this data:\n" + question +
            "\n\nIf the question asks to find or choose a schedule, return your answer in the following format:\n"
            "Index: <number>\nExplanation: <why this schedule is the best match>\n\n"
            "If the question only asks about the dataset (e.g. count, average, earliest time), just return a short factual response.\n"
            "Make sure to choose the right format based on the question.";

    std::string response = sendToModel(prompt, Models::OPENAI);
    std::cout << response << std::endl;

    std::string index = std::to_string(extractScheduleIndex(response));
    return {response, index};
}

int extractScheduleIndex(const std::string& text) {
    std::string lowered;
    lowered.reserve(text.size());

    // Convert to lowercase
    for (char c : text) {
        lowered += static_cast<char>(std::tolower(static_cast<unsigned char>(c)));
    }

    // Search for "index"
    size_t indexPos = lowered.find("index");
    if (indexPos == std::string::npos) {
        return -1;
    }

    // Move to character after "index"
    size_t pos = indexPos + 5;

    // Skip spaces and colons
    while (pos < lowered.size() && (lowered[pos] == ':' || std::isspace(static_cast<unsigned char>(lowered[pos])))) {
        pos++;
    }

    // Read number
    std::string numberStr;
    while (pos < lowered.size() && std::isdigit(static_cast<unsigned char>(lowered[pos]))) {
        numberStr += lowered[pos];
        pos++;
    }

    if (!numberStr.empty()) {
        try {
            return std::stoi(numberStr);
        } catch (...) {
            return -1;
        }
    }

    return -1;
}
