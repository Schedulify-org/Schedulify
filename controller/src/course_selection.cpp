#include "course_selection.h"

CourseSelectionController::CourseSelectionController(QObject *parent)
        : BaseController(parent)
        , m_courseModel(new CourseModel(this))
{
    // Initialize the course model with sample data
    m_courseModel->populateSampleData();
}

void CourseSelectionController::generateSchedules() {
    // Logic for generating schedules
    // ...

    // Navigate to schedules display screen
    goToScreen(QUrl(QStringLiteral("qrc:/schedules_display.qml")));
}
