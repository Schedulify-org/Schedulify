#include "course_selection.h"

CourseSelectionController::CourseSelectionController(QObject *parent)
        : ControllerManager(parent)
        , m_courseModel(new CourseModel(this))
        , m_selectedCoursesModel(new CourseModel(this))
        , m_filteredCourseModel(new CourseModel(this))
        , workerThread(nullptr)
{
    modelConnection = ModelFactory::createModel();
}

CourseSelectionController::~CourseSelectionController() {
    delete modelConnection;
}

void CourseSelectionController::initiateCoursesData(const vector<Course>& courses) {
    if (courses.empty()) {
        // Navigate to course selection screen
        Logger::get().logError("Empty courses vector provided to initiateCoursesData");
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

// CourseSelectionController.cpp
void CourseSelectionController::generateSchedules() {
    if (selectedCourses.empty()) {
        return;
    }

    // Create a worker thread for the operation
    workerThread = new QThread();
    auto* worker = new ScheduleGenerator(modelConnection, selectedCourses);
    worker->moveToThread(workerThread);

    // Connect signals/slots
    connect(workerThread, &QThread::started, worker, &ScheduleGenerator::generateSchedules);
    connect(worker, &ScheduleGenerator::schedulesGenerated, this, &CourseSelectionController::onSchedulesGenerated);
    connect(worker, &ScheduleGenerator::schedulesGenerated, workerThread, &QThread::quit);
    connect(workerThread, &QThread::finished, worker, &QObject::deleteLater);
    connect(workerThread, &QThread::finished, workerThread, &QObject::deleteLater);
    connect(this, &CourseSelectionController::abortScheduleGenerationRequested, worker, &ScheduleGenerator::abort);

    // Start thread before showing overlay
    workerThread->start();

    // Get the main QML engine
    auto* engine = qobject_cast<QQmlApplicationEngine*>(getEngine());
    if (!engine || engine->rootObjects().isEmpty()) {
        return;
    }

    // Show loading overlay with a slight delay
    QTimer::singleShot(100, this, [this, engine]() {
        if (workerThread && workerThread->isRunning()) {
            // Get root object
            QObject* rootObject = engine->rootObjects().first();
            if (!rootObject) {
                return;
            }

            // Call the QML function to show the overlay
            QMetaObject::invokeMethod(rootObject, "showLoadingOverlay",
                                      Q_ARG(QVariant, QVariant(true)));
        }
    });
}

// Handle the abort request from QML
void CourseSelectionController::abortGeneration() {
    emit abortScheduleGenerationRequested();
}

// Handle when schedules are generated
void CourseSelectionController::onSchedulesGenerated(std::vector<InformativeSchedule>* schedules) {
    // Get the main QML engine
    auto* engine = qobject_cast<QQmlApplicationEngine*>(getEngine());
    if (engine && !engine->rootObjects().isEmpty()) {
        // Hide the loading overlay
        QObject* rootObject = engine->rootObjects().first();
        QMetaObject::invokeMethod(rootObject, "showLoadingOverlay",
                                  Q_ARG(QVariant, QVariant(false)));
    }

    // Only process if we received schedules (not aborted)
    if (schedules) {
        auto* schedule_controller =
                qobject_cast<SchedulesDisplayController*>(findController("schedulesDisplayController"));

        schedule_controller->loadScheduleData(*schedules);

        // Navigate to schedules display screen
        goToScreen(QUrl(QStringLiteral("qrc:/schedules_display.qml")));
    }

    // Reset worker thread pointer
    workerThread = nullptr;
}

void CourseSelectionController::toggleCourseSelection(int index) {
    if (index < 0 || index >= static_cast<int>(allCourses.size())) {
        Logger::get().logError("Invalid selected course index");
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
        Logger::get().logError("Invalid selected course index");
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