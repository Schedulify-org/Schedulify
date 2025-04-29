#include "course_selection.h"

CourseSelectionController::CourseSelectionController(QObject *parent)
        : BaseController(parent)
        , m_courseModel(new CourseModel(this))
{
//    m_courseModel->populateCoursesData({});
}

void CourseSelectionController::initiateCoursesData(const vector<Course>& courses) {
    if (courses.empty()) {
        // Navigate to course selection screen
        goToScreen(QUrl(QStringLiteral("qrc:/file_input.qml")));
    } else {
        // Initialize the course model with the data
        m_courseModel->populateCoursesData({});
    }
}

void CourseSelectionController::generateSchedules() {
    // Logic for generating schedules
    // ...

    // Navigate to schedules display screen
    goToScreen(QUrl(QStringLiteral("qrc:/schedules_display.qml")));
}
