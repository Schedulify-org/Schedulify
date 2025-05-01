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
    } else {
        // Initialize the course model with the data
        allCourses = courses;
        m_courseModel->populateCoursesData(courses);
    }
}

void CourseSelectionController::generateSchedules() {
    Model model;

    vector<Course> selectedCourses = allCourses;

    //generate Courses vector
    auto* schedulePtr = static_cast<vector<Course>*>
        (model.executeOperation(ModelOperation::GENERATE_SCHEDULES, &selectedCourses, ""));

    // Navigate to schedules display screen
    goToScreen(QUrl(QStringLiteral("qrc:/schedules_display.qml")));
}
