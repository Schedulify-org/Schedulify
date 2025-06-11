#include "botAdapter.h"

vector<string> askModel(const string& question, const string& data, QuestionType type) {
    string prompt;

    cout << "start" << endl;
    cout << question << endl;

    switch (type) {
        case QuestionType::FIND_ME:
            prompt = "Here is a list of schedule metadata entries.\n" + data +
                     "\nPlease find the schedule that best fits the following request:\n" + question +
                     "\nReturn the best index and a short explanation why.";
            break;

        case QuestionType::TELL_ME:
            prompt = "Here is a list of schedule metadata entries.\n" + data +
                     "\nPlease answer this question about the dataset:\n" + question +
                     "\nReturn a short, factual response.";
            break;
    }

    string raw_response = sendToModel(prompt);
    cout << raw_response << endl;
    return getFormatedResponse(raw_response);
}

vector<string> getFormatedResponse(const string& raw_response) {
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

    vector<string> result;
    result.push_back(replyText);

    // Attempt to extract an index (if present)
    int index = -1;
    size_t pos = replyText.find("index");
    if (pos != string::npos) {
        try {
            string after = replyText.substr(pos);
            stringstream ss(after);
            string word;
            while (ss >> word) {
                try {
                    index = stoi(word);
                    break;
                } catch (...) {}
            }
        } catch (...) {}
    }

    result.push_back(index != -1 ? to_string(index) : "");

    return result;
}