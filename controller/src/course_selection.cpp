#include "course_selection.h"
#include <algorithm>
#include <QUrl>

CourseSelectionController::CourseSelectionController(QObject *parent)
        : ControllerManager(parent)
        , m_courseModel(new CourseModel(this))
        , m_selectedCoursesModel(new CourseModel(this))
        , m_filteredCourseModel(new CourseModel(this))
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

        // Initialize filtered courses with all courses
        filteredCourses = courses;
        filteredIndicesMap.clear();
        for (size_t i = 0; i < courses.size(); ++i) {
            filteredIndicesMap.push_back(i);
        }
        m_filteredCourseModel->populateCoursesData(filteredCourses, filteredIndicesMap);

        // Clear any previous selections
        selectedCourses.clear();
        selectedIndices.clear();
        m_selectedCoursesModel->populateCoursesData(selectedCourses);
    }
}

void CourseSelectionController::generateSchedules() {
    Model model;

    if (selectedCourses.empty()) {
        return;
    }

    //generate Schedule vector
    auto* schedulePtr = static_cast<vector<InformativeSchedule>*>
    (model.executeOperation(ModelOperation::GENERATE_SCHEDULES, &selectedCourses, ""));

    // Navigate to schedules display screen
    goToScreen(QUrl(QStringLiteral("qrc:/schedules_display.qml")));
}

void CourseSelectionController::toggleCourseSelection(int index) {
    if (index < 0 || index >= static_cast<int>(allCourses.size())) {
        qWarning() << "Invalid course index:" << index;
        return;
    }

    // Check if the course is already selected
    auto it = std::find(selectedIndices.begin(), selectedIndices.end(), index);

    if (it != selectedIndices.end()) {
        // Course is already selected, remove it
        int selectedIndex = std::distance(selectedIndices.begin(), it);
        selectedIndices.erase(it);
        selectedCourses.erase(selectedCourses.begin() + selectedIndex);
    } else {
        // Course is not selected, add it
        selectedIndices.push_back(index);
        selectedCourses.push_back(allCourses[index]);
    }

    // Update the selected courses model
    m_selectedCoursesModel->populateCoursesData(selectedCourses);

    // Emit our custom signal to notify QML of selection change
    emit selectionChanged();
}

void CourseSelectionController::deselectCourse(int index) {
    if (index < 0 || index >= static_cast<int>(selectedCourses.size())) {
        qWarning() << "Invalid selected course index:" << index;
        return;
    }

    // Find the corresponding index in the main course list
    int mainIndex = selectedIndices[index];

    // Remove from selected courses
    selectedCourses.erase(selectedCourses.begin() + index);
    selectedIndices.erase(selectedIndices.begin() + index);

    // Update the models
    m_selectedCoursesModel->populateCoursesData(selectedCourses);

    // Emit our custom signal to notify QML of selection change
    emit selectionChanged();
}

bool CourseSelectionController::isCourseSelected(int index) {
    return std::find(selectedIndices.begin(), selectedIndices.end(), index) != selectedIndices.end();
}

void CourseSelectionController::filterCourses(const QString& searchText) {
    if (searchText.isEmpty()) {
        resetFilter();
        return;
    }

    // Convert searchText to lowercase for case-insensitive search
    QString searchLower = searchText.toLower();

    // Clear previous filtered results
    filteredCourses.clear();
    filteredIndicesMap.clear();

    // Filter courses based on search text
    for (size_t i = 0; i < allCourses.size(); ++i) {
        const Course& course = allCourses[i];
        QString courseId = QString::fromStdString(course.raw_id).toLower();
        QString courseName = QString::fromStdString(course.name).toLower();
        QString teacherName = QString::fromStdString(course.teacher).toLower();

        // Check if search text is contained in any of the fields
        if (courseId.contains(searchLower) ||
            courseName.contains(searchLower) ||
            teacherName.contains(searchLower))
        {
            filteredCourses.push_back(course);
            filteredIndicesMap.push_back(i); // Store the original index
        }
    }

    // Update the filtered model
    m_filteredCourseModel->populateCoursesData(filteredCourses, filteredIndicesMap);
}

void CourseSelectionController::resetFilter() {
    // Reset filtered courses to show all
    filteredCourses = allCourses;
    filteredIndicesMap.clear();

    // Rebuild the index map
    for (size_t i = 0; i < allCourses.size(); ++i) {
        filteredIndicesMap.push_back(i);
    }

    // Update the filtered model
    m_filteredCourseModel->populateCoursesData(filteredCourses, filteredIndicesMap);
}