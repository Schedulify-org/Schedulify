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
    auto* coursesPtr = static_cast<vector<Course>*>
            (model.executeOperation(ModelOperation::GENERATE_COURSES, nullptr, filePath));

    vector<Course>& courses = *coursesPtr;

    // Navigate to course selection screen
    goToScreen(QUrl(QStringLiteral("qrc:/course_selection.qml")));

    if (!courses.empty()) {
        CourseSelectionController course_controller;
        course_controller.initiateCoursesData(courses);
    }
}
