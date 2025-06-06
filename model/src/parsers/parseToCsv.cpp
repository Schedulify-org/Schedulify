#include "parseToCsv.h"
#include <locale>
#include <codecvt>
#include <regex>

// Function to detect if text contains Hebrew characters
bool containsHebrew(const string& text) {
    // Check for Hebrew UTF-8 byte sequences
    for (size_t i = 0; i < text.length(); i++) {
        unsigned char c = static_cast<unsigned char>(text[i]);
        // Hebrew characters in UTF-8 start with 0xD7 (215) or 0xD6 (214)
        if (c == 0xD7 && i + 1 < text.length()) {
            unsigned char next = static_cast<unsigned char>(text[i + 1]);
            // Hebrew range: 0xD7 0x90 to 0xD7 0xAA
            if (next >= 0x90 && next <= 0xAA) {
                return true;
            }
        }
        // Also check for other Hebrew-related UTF-8 sequences
        if (c == 0xD6 && i + 1 < text.length()) {
            return true;
        }
    }
    return false;
}

// Function to get Hebrew translations
string getHebrewTranslation(const string& englishTerm) {
    static map<string, string> translations = {
        {"Building", "בניין"},
        {"Room", "חדר"},
        {"Lecture", "הרצאה"},
        {"Tutorial", "תרגיל"},
        {"Lab", "מעבדה"},
        {"Seminar", "סמינר"},
        {"Workshop", "סדנה"},
        {"Exam", "בחינה"},
        {"Exercise", "תרגיל"},
        {"Practicum", "פרקטיקום"},
        {"Class", "שיעור"}
    };

    auto it = translations.find(englishTerm);
    return (it != translations.end()) ? it->second : englishTerm;
}

int getHourFromTimeString(const string& timeStr) {
    try {
        return stoi(timeStr.substr(0, 2));
    } catch (const exception& e) {
        Logger::get().logError("Error parsing time string: " + timeStr + " - " + e.what());
        return 0;
    }
}

bool saveScheduleToCsv(const string& filePath, const InformativeSchedule& schedule) {
    // Open file in binary mode to have full control over encoding
    ofstream csvFile(filePath, ios::binary);
    if (!csvFile.is_open()) {
        Logger::get().logError("Error opening file: " + filePath);
        return false;
    }

    // Write UTF-8 BOM (Byte Order Mark) for proper Hebrew display
    // This tells programs like Excel that the file uses UTF-8 encoding
    csvFile.write("\xEF\xBB\xBF", 3);

    vector<string> days;
    for (const auto& day : schedule.week) {
        days.push_back(day.day);
    }

    const int minHour = 8;
    const int maxHour = 20;

    // Create a map to store all schedule data
    map<pair<int, string>, string> scheduleData;

    // Fill in the schedule data
    for (const auto& day : schedule.week) {
        for (const auto& item : day.day_items) {
            int startHour;
            int endHour;

            try {
                startHour = getHourFromTimeString(item.start);
                endHour = getHourFromTimeString(item.end);
            } catch (const exception& e) {
                Logger::get().logError("Failed to parse time for item: " + item.courseName + " - " + e.what());
                continue;
            }

            for (int hour = startHour; hour < endHour; hour++) {
                // Check if ANY content contains Hebrew to determine language
                bool isHebrew = containsHebrew(item.courseName) ||
                               containsHebrew(item.type) ||
                               containsHebrew(item.building) ||
                               containsHebrew(item.room) ||
                               containsHebrew(item.raw_id);

                // Debug: Always assume Hebrew for now to test translation
                // Remove this line once working properly

                // Create cell content with formatting
                stringstream cellContent;
                cellContent << item.courseName << " "
                            << item.raw_id << " - ";

                // Translate type if Hebrew
                if (isHebrew && item.type == "Lecture") {
                    cellContent << getHebrewTranslation("Lecture");
                } else if (isHebrew && item.type == "Tutorial") {
                    cellContent << getHebrewTranslation("Tutorial");
                } else {
                    cellContent << item.type;
                }

                cellContent << ", " << item.start << " - " << item.end << ", ";

                // Use Hebrew or English terms based on content language
                if (isHebrew) {
                    cellContent << getHebrewTranslation("Building") << ": " << item.building << ", "
                                << getHebrewTranslation("Room") << ": " << item.room;
                } else {
                    cellContent << "Building: " << item.building << ", Room: " << item.room;
                }

                // Store in map with (hour, day) as key
                scheduleData[{hour, day.day}] = cellContent.str();
            }
        }
    }

    // Write header row
    csvFile << "Hour/Day";
    for (const auto& day : days) {
        csvFile << "," << day;
    }
    csvFile << "\n";

    // Write data rows
    for (int hour = minHour; hour < maxHour; hour++) {
        stringstream timeStr;
        timeStr << setw(2) << setfill('0') << hour << ":00-"
                << setw(2) << setfill('0') << (hour + 1) << ":00";
        csvFile << timeStr.str();

        for (const auto& day : days) {
            csvFile << ",";

            auto key = make_pair(hour, day);
            if (scheduleData.find(key) != scheduleData.end()) {
                string content = scheduleData[key];

                // Escape quotes by doubling them for CSV format
                size_t pos = 0;
                while ((pos = content.find('\"', pos)) != string::npos) {
                    content.replace(pos, 1, "\"\"");
                    pos += 2;
                }

                // Always quote content to handle Hebrew text and special characters properly
                csvFile << "\"" << content << "\"";
            }
        }

        csvFile << "\n";
    }

    csvFile.close();
    Logger::get().logInfo("Schedule successfully saved to " + filePath);
    return true;
}

// Alternative function using wide characters for better Hebrew support
// Use this if the above doesn't work with your specific Hebrew text
bool saveScheduleToCsvWide(const string& filePath, const InformativeSchedule& schedule) {
    // Convert string to wstring for Hebrew support
    auto stringToWstring = [](const string& str) -> wstring {
        wstring_convert<codecvt_utf8<wchar_t>, wchar_t> converter;
        return converter.from_bytes(str);
    };

    // Set up wide file stream with UTF-8 locale
    locale utf8_locale(locale(), new codecvt_utf8<wchar_t>);
    wofstream csvFile(filePath);
    csvFile.imbue(utf8_locale);

    if (!csvFile.is_open()) {
        Logger::get().logError("Error opening file: " + filePath);
        return false;
    }

    vector<wstring> days;
    for (const auto& day : schedule.week) {
        days.push_back(stringToWstring(day.day));
    }

    const int minHour = 8;
    const int maxHour = 20;

    // Create a map to store all schedule data (using wide strings)
    map<pair<int, wstring>, wstring> scheduleData;

    // Fill in the schedule data
    for (const auto& day : schedule.week) {
        wstring wDay = stringToWstring(day.day);

        for (const auto& item : day.day_items) {
            int startHour;
            int endHour;

            try {
                startHour = getHourFromTimeString(item.start);
                endHour = getHourFromTimeString(item.end);
            } catch (const exception& e) {
                Logger::get().logError("Failed to parse time for item: " + item.courseName + " - " + e.what());
                continue;
            }

            for (int hour = startHour; hour < endHour; hour++) {
                // Check if course name contains Hebrew to determine language
                bool isHebrew = false;
                wstring wCourseName = stringToWstring(item.courseName);
                wstring wType = stringToWstring(item.type);
                wstring wBuilding = stringToWstring(item.building);

                // Check for Hebrew characters in wide strings
                for (wchar_t c : wCourseName + wType + wBuilding) {
                    if (c >= 0x05D0 && c <= 0x05EA) {  // Hebrew Unicode range
                        isHebrew = true;
                        break;
                    }
                }

                // Create cell content with formatting
                wstringstream cellContent;
                cellContent << wCourseName << L" "
                            << stringToWstring(item.raw_id) << L" - " << wType << L", "
                            << stringToWstring(item.start) << L" - " << stringToWstring(item.end) << L", ";

                // Use Hebrew or English terms based on content language
                if (isHebrew) {
                    cellContent << stringToWstring(getHebrewTranslation("Building")) << L": " << wBuilding << L", "
                                << stringToWstring(getHebrewTranslation("Room")) << L": " << stringToWstring(item.room);
                } else {
                    cellContent << L"Building: " << wBuilding << L", Room: " << stringToWstring(item.room);
                }

                // Store in map with (hour, day) as key
                scheduleData[{hour, wDay}] = cellContent.str();
            }
        }
    }

    // Write header row
    csvFile << L"Hour/Day";
    for (const auto& day : days) {
        csvFile << L"," << day;
    }
    csvFile << L"\n";

    // Write data rows
    for (int hour = minHour; hour < maxHour; hour++) {
        wstringstream timeStr;
        timeStr << setw(2) << setfill(L'0') << hour << L":00-"
                << setw(2) << setfill(L'0') << (hour + 1) << L":00";
        csvFile << timeStr.str();

        for (const auto& day : days) {
            csvFile << L",";

            auto key = make_pair(hour, day);
            if (scheduleData.find(key) != scheduleData.end()) {
                wstring content = scheduleData[key];

                // Escape quotes by doubling them for CSV format
                size_t pos = 0;
                while ((pos = content.find(L'\"', pos)) != wstring::npos) {
                    content.replace(pos, 1, L"\"\"");
                    pos += 2;
                }

                // Always quote content to handle Hebrew text properly
                csvFile << L"\"" << content << L"\"";
            }
        }

        csvFile << L"\n";
    }

    csvFile.close();
    Logger::get().logInfo("Schedule successfully saved to " + filePath);
    return true;
}