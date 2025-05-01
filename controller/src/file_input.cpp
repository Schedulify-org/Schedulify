#include "file_input.h"
#include <filesystem>

FileInputController::FileInputController(QObject *parent)
        : ControllerManager(parent){}

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

    auto* course_controller =
            qobject_cast<CourseSelectionController*>(findController("courseSelectionController"));


    if (course_controller && !courses.empty()) {
        // Initialize the course data first
        course_controller->initiateCoursesData(courses);

        // Then navigate to course selection screen
        goToScreen(QUrl(QStringLiteral("qrc:/course_selection.qml")));
    } else {
        qWarning() << "Failed to find courseSelectionController!";
    }
}
