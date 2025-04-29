#include "file_input.h"
#include <filesystem>

FileInputController::FileInputController(QObject *parent)
        : BaseController(parent){}

void FileInputController::handleUploadAndContinue() {
    Model model;

    //temp path for tests
    namespace fs = std::filesystem;
    fs::path main_path = fs::current_path().parent_path();
    string filePath = main_path.string() + COURSEDBINPUT;

    //generate Courses vector
    auto* courses = static_cast<vector<Course>*>
            (model.executeOperation(ModelOperation::GENERATE_COURSES, nullptr, filePath));
    if (!courses->empty()) {
        // Navigate to course selection screen
        goToScreen(QUrl(QStringLiteral("qrc:/course_selection.qml")));
    }
}
