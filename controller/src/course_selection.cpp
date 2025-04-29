#include "course_selection.h"

CourseSelectionController::CourseSelectionController(QObject *parent)
        : BaseController(parent)
        , m_courseModel(new CourseModel(this))
{
}

void CourseSelectionController::initialize() {
    initiateCoursesData();
}

void CourseSelectionController::initiateCoursesData() {
    Model model;

    //get courses
    auto* coursesPtr = static_cast<vector<Course>*>
    (model.executeOperation(ModelOperation::GET_COURSES, nullptr, ""));

    // Check if pointer is valid before dereferencing
    if (!coursesPtr) {
        goToScreen(QUrl(QStringLiteral("qrc:/file_input.qml")));
        return;
    }

    vector<Course>& courses = *coursesPtr;

    if (courses.empty()) {
        // Navigate to course selection screen
        goToScreen(QUrl(QStringLiteral("qrc:/file_input.qml")));
    } else {
        // Initialize the course model with the data
        m_courseModel->populateCoursesData(courses);
    }
}

void CourseSelectionController::generateSchedules() {
    // Logic for generating schedules
    // ...

    // Navigate to schedules display screen
    goToScreen(QUrl(QStringLiteral("qrc:/schedules_display.qml")));
}
