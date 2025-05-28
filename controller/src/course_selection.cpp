#include "main_controller.h"
#include "course_selection.h"

CourseSelectionController::CourseSelectionController(QObject *parent)
        : ControllerManager(parent)
        , m_courseModel(new CourseModel(this))
        , m_selectedCoursesModel(new CourseModel(this))
        , m_filteredCourseModel(new CourseModel(this))
        , m_blocksModel(new CourseModel(this))
        , workerThread(nullptr)
{
    modelConnection = ModelAccess::getModel();
}

CourseSelectionController::~CourseSelectionController() {
    delete modelConnection;
}

void CourseSelectionController::initiateCoursesData(const vector<Course>& courses) {
    if (courses.empty()) {
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

        // Clear block times
        userBlockTimes.clear();
        blockTimes.clear();
        updateBlockTimesModel();
    }
}

void CourseSelectionController::addBlockTime(const QString& day, const QString& startTime, const QString& endTime) {
    // Check if this block time already exists
    for (const auto& blockTime : userBlockTimes) {
        if (blockTime.day == day && blockTime.startTime == startTime && blockTime.endTime == endTime) {
            emit errorMessage("This block time already exists");
            return;
        }
    }

    // Validate time format and logic
    if (startTime >= endTime) {
        emit errorMessage("Start time must be before end time");
        return;
    }

    // Add the new block time
    userBlockTimes.emplace_back(day, startTime, endTime);
    updateBlockTimesModel();

    emit blockTimesChanged();
}

void CourseSelectionController::removeBlockTime(int index) {
    if (index < 0 || index >= static_cast<int>(userBlockTimes.size())) {
        Logger::get().logError("Invalid block time index for removal");
        return;
    }

    userBlockTimes.erase(userBlockTimes.begin() + index);
    updateBlockTimesModel();

    emit blockTimesChanged();
}

void CourseSelectionController::clearAllBlockTimes() {
    userBlockTimes.clear();
    updateBlockTimesModel();
    emit blockTimesChanged();
}

void CourseSelectionController::updateBlockTimesModel() {
    blockTimes.clear();

    for (size_t i = 0; i < userBlockTimes.size(); ++i) {
        const BlockTime& blockTime = userBlockTimes[i];
        Course blockCourse = createBlockTimeCourse(blockTime, static_cast<int>(i) + 90000);
        blockTimes.push_back(blockCourse);
    }

    m_blocksModel->populateCoursesData(blockTimes);
}

Course CourseSelectionController::createBlockTimeCourse(const BlockTime& blockTime, int id) {
    // Create a Course object to represent the block time for display purposes
    // We'll use the course fields for display:
    // - raw_id will show the time range
    // - name will show "Blocked Time"
    // - teacher can show the day for additional info

    Course blockCourse;
    blockCourse.id = id;
    blockCourse.raw_id = (blockTime.startTime + " - " + blockTime.endTime).toStdString();
    blockCourse.name = "Blocked Time"; // Fixed: should always be "Blocked Time"
    blockCourse.teacher = blockTime.day.toStdString(); // Day info moved to teacher field

    // Clear all group vectors
    blockCourse.Lectures.clear();
    blockCourse.Tirgulim.clear();
    blockCourse.labs.clear();
    blockCourse.blocks.clear();

    // Create a block group with the blocked time session
    Group blockGroup = createBlockGroup(blockTime);
    blockCourse.blocks.push_back(blockGroup);

    return blockCourse;
}

Group CourseSelectionController::createBlockGroup(const BlockTime& blockTime) {
    Group blockGroup;
    blockGroup.type = SessionType::BLOCK;

    // Create a session that represents the blocked time
    Session blockSession;
    blockSession.day_of_week = getDayNumber(blockTime.day);
    blockSession.start_time = blockTime.startTime.toStdString();
    blockSession.end_time = blockTime.endTime.toStdString();
    blockSession.building_number = "BLOCKED";
    blockSession.room_number = "BLOCK";

    blockGroup.sessions.push_back(blockSession);

    return blockGroup;
}

int CourseSelectionController::getDayNumber(const QString& dayName) {
    if (dayName == "Sunday") return 1;
    if (dayName == "Monday") return 2;
    if (dayName == "Tuesday") return 3;
    if (dayName == "Wednesday") return 4;
    if (dayName == "Thursday") return 5;
    if (dayName == "Friday") return 6;
    if (dayName == "Saturday") return 7;
    return 1;
}

void CourseSelectionController::generateSchedules() {
    if (selectedCourses.empty()) {
        emit errorMessage("Please select at least one course");
        return;
    }

    // Create a worker thread for the operation
    workerThread = new QThread();

    // Combine selected courses with block times
    vector<Course> coursesToProcess = selectedCourses;

    // Add user-defined block times as courses
    for (size_t i = 0; i < userBlockTimes.size(); ++i) {
        const auto& blockTime = userBlockTimes[i];
        Course blockCourse = createBlockTimeCourse(blockTime, 99000 + static_cast<int>(i));
        coursesToProcess.push_back(blockCourse);
    }

    auto* worker = new ScheduleGenerator(modelConnection, coursesToProcess);
    worker->moveToThread(workerThread);

    // Connect signals/slots
    connect(workerThread, &QThread::started, worker, &ScheduleGenerator::generateSchedules);
    connect(worker, &ScheduleGenerator::schedulesGenerated, this, &CourseSelectionController::onSchedulesGenerated);
    connect(worker, &ScheduleGenerator::schedulesGenerated, workerThread, &QThread::quit);
    connect(workerThread, &QThread::finished, worker, &QObject::deleteLater);
    connect(workerThread, &QThread::finished, workerThread, &QObject::deleteLater);

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

void CourseSelectionController::onSchedulesGenerated(vector<InformativeSchedule>* schedules) {
    // Get the main QML engine
    auto* engine = qobject_cast<QQmlApplicationEngine*>(getEngine());
    if (engine && !engine->rootObjects().isEmpty()) {
        // Hide the loading overlay
        QObject* rootObject = engine->rootObjects().first();
        QMetaObject::invokeMethod(rootObject, "showLoadingOverlay",
                                  Q_ARG(QVariant, QVariant(false)));
    }

    // Only process if we received schedules (not aborted)
    if (schedules && !schedules->empty()) {
        auto* schedule_controller =
                qobject_cast<SchedulesDisplayController*>(findController("schedulesDisplayController"));

        schedule_controller->loadScheduleData(*schedules);

        // Navigate to schedules display screen
        goToScreen(QUrl(QStringLiteral("qrc:/schedules_display.qml")));
    } else {
        emit errorMessage("There are no valid schedules for your selected courses and block times");
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
    auto it = find(selectedIndices.begin(), selectedIndices.end(), index);

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

    // Remove from selected courses
    selectedCourses.erase(selectedCourses.begin() + index);
    selectedIndices.erase(selectedIndices.begin() + index);

    // Update the selected courses model
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
            filteredIndicesMap.push_back(static_cast<int>(i)); // Store the original index
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
        filteredIndicesMap.push_back(static_cast<int>(i));
    }

    // Update the filtered model
    m_filteredCourseModel->populateCoursesData(filteredCourses, filteredIndicesMap);
}