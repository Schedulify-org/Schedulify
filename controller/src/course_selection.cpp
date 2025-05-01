#include "course_selection.h"

CourseSelectionController::CourseSelectionController(QObject *parent)
        : ControllerManager(parent)
        , m_courseModel(new CourseModel(this))
{
}

void CourseSelectionController::initiateCoursesData(const vector<Course>& courses) {
    if (courses.empty()) {
        // Navigate to course selection screen
        qWarning() << "Warning: Empty courses vector provided to initiateCoursesData";
        navigateBack();
    } else {
        // Initialize the course model with the data
        m_courseModel->populateCoursesData(courses);
    }
}

void CourseSelectionController::generateSchedules() {
    // Navigate to schedules display screen
    goToScreen(QUrl(QStringLiteral("qrc:/schedules_display.qml")));
}
