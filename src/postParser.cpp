#include "../include/postParser.h"

void exportCompactJson(const vector<Schedule>& schedules, const string& outputPath) {
    ofstream outFile(outputPath);
    if (!outFile.is_open()) {
        cerr << "Failed to open output file: " << outputPath << endl;
        return;
    }

    outFile << "[";

    for (size_t i = 0; i < schedules.size(); ++i) {
        const auto& schedule = schedules[i];
        outFile << "{\"schedule_number\":" << (i + 1) << ",\"courses\":[";

        for (size_t j = 0; j < schedule.selections.size(); ++j) {
            const auto& cs = schedule.selections[j];
            outFile << "{\"course_id\":" << cs.courseId;

            if (cs.lecture) {
                outFile << ",\"lecture\":{"
                        << "\"day\":" << cs.lecture->day_of_week
                        << ",\"start\":\"" << cs.lecture->start_time << "\""
                        << ",\"end\":\"" << cs.lecture->end_time << "\""
                        << ",\"building\":\"" << cs.lecture->building_number << "\""
                        << ",\"room\":\"" << cs.lecture->room_number << "\"}";
            }

            if (cs.tutorial) {
                outFile << ",\"tutorial\":{"
                        << "\"day\":" << cs.tutorial->day_of_week
                        << ",\"start\":\"" << cs.tutorial->start_time << "\""
                        << ",\"end\":\"" << cs.tutorial->end_time << "\""
                        << ",\"building\":\"" << cs.tutorial->building_number << "\""
                        << ",\"room\":\"" << cs.tutorial->room_number << "\"}";
            }

            if (cs.lab) {
                outFile << ",\"lab\":{"
                        << "\"day\":" << cs.lab->day_of_week
                        << ",\"start\":\"" << cs.lab->start_time << "\""
                        << ",\"end\":\"" << cs.lab->end_time << "\""
                        << ",\"building\":\"" << cs.lab->building_number << "\""
                        << ",\"room\":\"" << cs.lab->room_number << "\"}";
            }

            outFile << "}";

            if (j != schedule.selections.size() - 1)
                outFile << ",";
        }

        outFile << "]}";
        if (i != schedules.size() - 1)
            outFile << ",";
    }

    outFile << "]";
    outFile.close();
}