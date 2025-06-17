#include "main_controller.h"
#include "course_selection.h"
CourseSelectionController::CourseSelectionController(QObject *parent)
        : ControllerManager(parent)
        , m_courseModel(new CourseModel(this))
        , m_selectedCoursesModel(new CourseModel(this))
        , m_filteredCourseModel(new CourseModel(this))
        , m_blocksModel(new CourseModel(this))
        , workerThread(nullptr)
        , validatorThread(nullptr)
        , m_validationInProgress(false)
{
    modelConnection = ModelAccess::getModel();
}

CourseSelectionController::~CourseSelectionController() {

    cleanupValidatorThread();

    if (workerThread) {
        if (workerThread->isRunning()) {
            workerThread->quit();
            if (!workerThread->wait(3000)) {
                workerThread->terminate();
                workerThread->wait(1000);
            }
        }
        workerThread = nullptr;
    }

    if (modelConnection) {
        modelConnection = nullptr;
    }
}

void CourseSelectionController::setValidationInProgress(bool inProgress) {
    if (m_validationInProgress != inProgress) {
        m_validationInProgress = inProgress;
        emit validationStateChanged();
    }
}

void CourseSelectionController::setValidationErrors(const QStringList& errors) {
    if (m_validationErrors != errors) {
        m_validationErrors = errors;
        emit validationStateChanged();
    }
}

void CourseSelectionController::initiateCoursesData(const vector<Course>& courses) {
    try {
        if (courses.empty()) {
            Logger::get().logError("Empty courses vector provided");
            setValidationErrors(QStringList{"No courses found in file"});
            return;
        }

        cleanupValidatorThread();

        setValidationInProgress(true);

        allCourses = courses;
        m_courseModel->populateCoursesData(courses);

        filteredCourses = courses;
        filteredIndicesMap.clear();
        for (size_t i = 0; i < courses.size(); ++i) {
            filteredIndicesMap.push_back(static_cast<int>(i));
        }
        m_filteredCourseModel->populateCoursesData(filteredCourses, filteredIndicesMap);

        selectedCourses.clear();
        selectedIndices.clear();
        m_selectedCoursesModel->populateCoursesData(selectedCourses);
        userBlockTimes.clear();
        blockTimes.clear();
        updateBlockTimesModel();

        setValidationErrors(QStringList());

        int timeoutMs = std::min(VALIDATION_TIMEOUT_MS,
                                 static_cast<int>(courses.size() * 100 + 10000));

        QTimer::singleShot(100, this, [this, courses, timeoutMs]() {
            validateCourses(courses, timeoutMs);
        });

    } catch (const std::exception& e) {
        Logger::get().logError("Exception in initiateCoursesData: " + std::string(e.what()));
        setValidationInProgress(false);
    } catch (...) {
        Logger::get().logError("Unknown exception in initiateCoursesData");
        setValidationInProgress(false);
    }
}

void CourseSelectionController::validateCourses(const vector<Course>& courses, int timeoutMs) {
    if (validatorThread && validatorThread->isRunning()) {
        return;
    }

    cleanupValidatorThread();
    validationCompleted = false;
    setValidationInProgress(true);

    Logger::get().logInfo("Starting validation");

    // Create validation thread with safety limits
    validatorThread = new QThread(this);
    auto* worker = new CourseValidator(modelConnection, courses);
    worker->moveToThread(validatorThread);

    // Setup timeout with custom duration
    setupValidationTimeout(timeoutMs);

    // Connect signals
    connect(validatorThread, &QThread::started, worker, &CourseValidator::validateCourses, Qt::QueuedConnection);
    connect(worker, &CourseValidator::coursesValidated, this, &CourseSelectionController::onCoursesValidated, Qt::QueuedConnection);
    connect(worker, &CourseValidator::coursesValidated, worker, &QObject::deleteLater, Qt::QueuedConnection);
    connect(worker, &CourseValidator::coursesValidated, validatorThread, &QThread::quit, Qt::QueuedConnection);
    connect(worker, &CourseValidator::coursesValidated, this, &CourseSelectionController::cleanupValidation, Qt::QueuedConnection);
    connect(validatorThread, &QThread::finished, validatorThread, &QObject::deleteLater, Qt::QueuedConnection);

    connect(validatorThread, &QThread::finished, [this]() {
        validatorThread = nullptr;
    });

    validatorThread->start();
}

void CourseSelectionController::setupValidationTimeout(int timeoutMs) {
    if (validationTimeoutTimer) {
        validationTimeoutTimer->stop();
        validationTimeoutTimer->deleteLater();
    }

    validationTimeoutTimer = new QTimer(this);
    validationTimeoutTimer->setSingleShot(true);
    validationTimeoutTimer->setInterval(timeoutMs);

    connect(validationTimeoutTimer, &QTimer::timeout, this, &CourseSelectionController::onValidationTimeout);
    validationTimeoutTimer->start();
}

void CourseSelectionController::onValidationTimeout() {
    if (validationCompleted) {
        return;
    }

    validationCompleted = true;
    setValidationInProgress(false);

    if (validatorThread && validatorThread->isRunning()) {
        auto workers = validatorThread->findChildren<CourseValidator*>();
        for (auto* worker : workers) {
            worker->cancelValidation();
        }

        QTimer::singleShot(2000, this, [this]() {
            if (validatorThread && validatorThread->isRunning()) {
                validatorThread->quit();

                QTimer::singleShot(3000, this, [this]() {
                    if (validatorThread && validatorThread->isRunning()) {
                        validatorThread->terminate();
                        validatorThread->wait(2000);
                    }
                });
            }
        });
    }

    // Set error state in UI
    setValidationErrors(QStringList{
            "[System] Validation timed out",
            "The course file may be too large or contain complex conflicts",
            "Try using a smaller file or contact support if this continues"
    });

    cleanupValidation();
}

void CourseSelectionController::onCoursesValidated(vector<string>* errors) {
    if (validationTimeoutTimer && validationTimeoutTimer->isActive()) {
        validationTimeoutTimer->stop();
    }

    if (validationCompleted) {
        if (errors) {
            delete errors;
        }
        return;
    }

    validationCompleted = true;
    setValidationInProgress(false);

    QStringList qmlErrors;

    try {
        if (!errors) {
            Logger::get().logError("Received null errors pointer");
            qmlErrors.append("[System] Validation failed - internal error");
        } else {

            for (size_t i = 0; i < errors->size(); ++i) {
                try {
                    const string& message = errors->at(i);
                    qmlErrors.append(QString::fromStdString(message));
                } catch (const std::exception& e) {
                }
            }
        }

    } catch (const std::exception& e) {
        qmlErrors.clear();
    } catch (...) {
        Logger::get().logError("Unknown exception while processing validation results");
        qmlErrors.clear();
    }

    setValidationErrors(qmlErrors);

    Logger::get().logInfo("Validation processing completed safely");
}

void CourseSelectionController::cleanupValidation() {
    if (validationTimeoutTimer && validationTimeoutTimer->isActive()) {
        validationTimeoutTimer->stop();
    }

    if (validationTimeoutTimer) {
        validationTimeoutTimer->deleteLater();
        validationTimeoutTimer = nullptr;
    }

    validationCompleted = false;

    setValidationInProgress(false);
}

void CourseSelectionController::cleanupValidatorThread() {
    if (!validatorThread) {
        return;
    }

    try {
        auto workers = validatorThread->findChildren<CourseValidator*>();
        for (auto* worker : workers) {
            worker->cancelValidation();
        }

        if (!validatorThread->isRunning()) {
            validatorThread->deleteLater();
            validatorThread = nullptr;
            return;
        }

        validatorThread->quit();

        if (validatorThread->wait(5000)) {
            Logger::get().logInfo("Thread quit gracefully");
        } else {
            Logger::get().logWarning("Thread didn't quit gracefully, forcing termination");
            validatorThread->terminate();
            if (validatorThread->wait(3000)) {
                Logger::get().logInfo("Thread terminated successfully");
            } else {
                Logger::get().logError("Thread termination failed, emergency cleanup");
            }
        }

        validatorThread->deleteLater();
        validatorThread = nullptr;

    } catch (const std::exception& e) {
        validatorThread = nullptr;
    } catch (...) {
        validatorThread = nullptr;
    }

    if (validationTimeoutTimer) {
        validationTimeoutTimer->stop();
        validationTimeoutTimer->deleteLater();
        validationTimeoutTimer = nullptr;
    }

    setValidationInProgress(false);
    validationCompleted = false;
}

void CourseSelectionController::addBlockTime(const QString& day, const QString& startTime, const QString& endTime) {
    // Validate time format and logic first
    if (startTime >= endTime) {
        emit errorMessage("Start time must be before end time");
        return;
    }

    // Helper function to convert time string to minutes for easier comparison
    auto timeToMinutes = [](const QString& time) -> int {
        QStringList parts = time.split(":");
        if (parts.size() != 2) return -1;
        int hours = parts[0].toInt();
        int minutes = parts[1].toInt();
        return hours * 60 + minutes;
    };

    int newStartMinutes = timeToMinutes(startTime);
    int newEndMinutes = timeToMinutes(endTime);

    if (newStartMinutes == -1 || newEndMinutes == -1) {
        emit errorMessage("Invalid time format");
        return;
    }

    // Check for overlaps with existing block times on the same day
    for (const auto& blockTime : userBlockTimes) {
        if (blockTime.day == day) {
            int existingStartMinutes = timeToMinutes(blockTime.startTime);
            int existingEndMinutes = timeToMinutes(blockTime.endTime);

            // Check if the new time block overlaps with the existing one
            // Two time blocks overlap if:
            // 1. New start time is before existing end time AND
            // 2. New end time is after existing start time
            if (newStartMinutes < existingEndMinutes && newEndMinutes > existingStartMinutes) {
                emit errorMessage(QString("Time block overlaps with existing block on %1 (%2 - %3)")
                                          .arg(day)
                                          .arg(blockTime.startTime)
                                          .arg(blockTime.endTime));
                return;
            }
        }
    }

    // If we get here, there's no overlap, so add the new block time
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

Course CourseSelectionController::createSingleBlockTimeCourse() {
    Course blockCourse;
    blockCourse.id = 90000; // Fixed ID for the single block course
    blockCourse.raw_id = "TIME_BLOCKS";
    blockCourse.name = "Time Block";
    blockCourse.teacher = "System Generated";

    // Clear all session type vectors
    blockCourse.Lectures.clear();
    blockCourse.Tirgulim.clear();
    blockCourse.labs.clear();
    blockCourse.blocks.clear();

    // Create a single group containing all block times
    Group blockGroup;
    blockGroup.type = SessionType::BLOCK;

    // Add all user block times as sessions to this single group
    for (const auto& blockTime : userBlockTimes) {
        Session blockSession;
        blockSession.day_of_week = getDayNumber(blockTime.day);
        blockSession.start_time = blockTime.startTime.toStdString();
        blockSession.end_time = blockTime.endTime.toStdString();
        blockSession.building_number = "BLOCKED";
        blockSession.room_number = "BLOCK";

        blockGroup.sessions.push_back(blockSession);
    }

    // Add the group to the course's blocks
    blockCourse.blocks.push_back(blockGroup);

    return blockCourse;
}

void CourseSelectionController::updateBlockTimesModel() {
    blockTimes.clear();

    for (size_t i = 0; i < userBlockTimes.size(); ++i) {
        const BlockTime& blockTime = userBlockTimes[i];
        Course blockCourse;
        blockCourse.id = static_cast<int>(i) + 90000;
        blockCourse.raw_id = (blockTime.startTime + " - " + blockTime.endTime).toStdString();
        blockCourse.name = "Blocked Time";
        blockCourse.teacher = blockTime.day.toStdString(); // Store day in teacher field for display

        blockCourse.Lectures.clear();
        blockCourse.Tirgulim.clear();
        blockCourse.labs.clear();
        blockCourse.blocks.clear();

        // Create a group for this block time
        Group blockGroup;
        blockGroup.type = SessionType::BLOCK;

        Session blockSession;
        blockSession.day_of_week = getDayNumber(blockTime.day);
        blockSession.start_time = blockTime.startTime.toStdString();
        blockSession.end_time = blockTime.endTime.toStdString();
        blockSession.building_number = "BLOCKED";
        blockSession.room_number = "BLOCK";

        blockGroup.sessions.push_back(blockSession);
        blockCourse.blocks.push_back(blockGroup);

        blockTimes.push_back(blockCourse);
    }
    m_blocksModel->populateCoursesData(blockTimes);
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

    // Add single block times course if there are any block times
    if (!userBlockTimes.empty()) {
        Course blockCourse = createSingleBlockTimeCourse();
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
        int selectedIndex = std::distance(selectedIndices.begin(), it);
        selectedIndices.erase(it);
        selectedCourses.erase(selectedCourses.begin() + selectedIndex);
    } else {
        selectedIndices.push_back(index);
        selectedCourses.push_back(allCourses[index]);
    }

    // Update the selected courses model
    m_selectedCoursesModel->populateCoursesData(selectedCourses);

    emit selectionChanged();
}

void CourseSelectionController::deselectCourse(int index) {
    if (index < 0 || index >= static_cast<int>(selectedCourses.size())) {
        Logger::get().logError("Invalid selected course index");
        return;
    }

    selectedCourses.erase(selectedCourses.begin() + index);
    selectedIndices.erase(selectedIndices.begin() + index);

    m_selectedCoursesModel->populateCoursesData(selectedCourses);

    emit selectionChanged();
}

bool CourseSelectionController::isCourseSelected(int index) {
    return std::find(selectedIndices.begin(), selectedIndices.end(), index) != selectedIndices.end();
}

void CourseSelectionController::filterCourses(const QString& searchText) {
    currentSearchText = searchText;
    applyFilters();
}

void CourseSelectionController::resetFilter() {
    currentSearchText.clear();
    currentSemesterFilter = "ALL";
    applyFilters();
}

void CourseSelectionController::createNewCourse(const QString& courseName, const QString& courseId,
                                                const QString& teacherName, const QVariantList& sessionGroups) {

    for (const auto& course : allCourses) {
        if (QString::fromStdString(course.raw_id) == courseId) {
            emit errorMessage("Course ID already exists");
            return;
        }
    }

    Course newCourse = createCourseFromData(courseName, courseId, teacherName, sessionGroups);
    Logger::get().logInfo("Created new course with ID: " + to_string(newCourse.id) +
                          ", name: " + newCourse.name + ", raw_id: " + newCourse.raw_id);

    // Add to allCourses
    allCourses.push_back(newCourse);

    // Update the main course model
    m_courseModel->populateCoursesData(allCourses);

    // Update filtered courses and indices map
    QString searchLower = currentSearchText.toLower();
    QString courseIdLower = courseId.toLower();
    QString courseNameLower = courseName.toLower();
    QString teacherNameLower = teacherName.toLower();

    bool shouldIncludeInFilter = searchLower.isEmpty() ||
                                 courseIdLower.contains(searchLower) ||
                                 courseNameLower.contains(searchLower) ||
                                 teacherNameLower.contains(searchLower);

    if (shouldIncludeInFilter) {
        filteredCourses.push_back(newCourse);
        filteredIndicesMap.push_back(static_cast<int>(allCourses.size() - 1));
        m_filteredCourseModel->populateCoursesData(filteredCourses, filteredIndicesMap);
    }

    Logger::get().logInfo("New course created: " + courseName.toStdString() + ", "  + courseId.toStdString());

    cleanupValidatorThread();

    setValidationInProgress(true);

    setValidationErrors(QStringList());

    vector <Course> coursesToValidate = allCourses;

    int timeoutMs = std::min(VALIDATION_TIMEOUT_MS,
                             static_cast<int>(coursesToValidate.size() * 100 + 10000)); // 100ms per course + 10s base

    QTimer::singleShot(100, this, [this, coursesToValidate, timeoutMs]() {
        validateCourses(coursesToValidate, timeoutMs);
    });
}

Course CourseSelectionController::createCourseFromData(const QString& courseName, const QString& courseId,
                                                       const QString& teacherName, const QVariantList& sessionGroups) {
    Course course;
    course.id = courseId.toInt();
    course.raw_id = courseId.toStdString();
    course.name = courseName.toStdString();
    course.teacher = teacherName.toStdString();

    // Clear all session vectors
    course.Lectures.clear();
    course.Tirgulim.clear();
    course.labs.clear();
    course.blocks.clear();

    // Process session groups
    for (const QVariant& groupVar : sessionGroups) {
        QVariantMap groupMap = groupVar.toMap();
        string groupType = groupMap["type"].toString().toStdString();
        QVariantList sessions = groupMap["sessions"].toList();

        Group group;

        // Set group type
        if (groupType == "Lecture") {
            group.type = SessionType::LECTURE;
        } else if (groupType == "Tutorial") {
            group.type = SessionType::TUTORIAL;
        } else if (groupType == "Lab") {
            group.type = SessionType::LAB;
        } else {
            group.type = SessionType::LECTURE;
        }

        Logger::get().logInfo("parsing group from type: " + groupMap["type"].toString().toStdString());

        // Process sessions for this group
        for (const QVariant& sessionVar : sessions) {
            Logger::get().logInfo("A");
            QVariantMap sessionMap = sessionVar.toMap();
            Logger::get().logInfo("B");

            Session session;
            Logger::get().logInfo("C");

            session.day_of_week = getDayNumber(sessionMap["day"].toString());
            Logger::get().logInfo("D");


            // Ensure proper time format
            QString startTime = sessionMap["startTime"].toString();
            QString endTime = sessionMap["endTime"].toString();

            Logger::get().logInfo("E");


            // Add validation and formatting
            if (!startTime.contains(":")) {
                startTime = startTime + ":00";
            }
            if (!endTime.contains(":")) {
                endTime = endTime + ":00";
            }

            Logger::get().logInfo("F");


            session.start_time = startTime.toStdString();
            session.end_time = endTime.toStdString();
            session.building_number = sessionMap["building"].toString().toStdString();
            session.room_number = sessionMap["room"].toString().toStdString();

            Logger::get().logInfo("Manual course session created: Day=" + std::to_string(session.day_of_week) +
                                  ", Start=" + session.start_time +
                                  ", End=" + session.end_time +
                                  ", Building=" + session.building_number +
                                  ", Room=" + session.room_number);

            group.sessions.push_back(session);
        }

        // Add group to appropriate vector
        if (group.type == SessionType::LECTURE) {
            course.Lectures.push_back(group);
        } else if (group.type == SessionType::TUTORIAL) {
            course.Tirgulim.push_back(group);
        } else if (group.type == SessionType::LAB) {
            course.labs.push_back(group);
        }
    }

    return course;
}void CourseSelectionController::filterBySemester(const QString& semester) {
    currentSemesterFilter = semester;
    applyFilters();
}

void CourseSelectionController::applyFilters() {
    filteredCourses.clear();
    filteredIndicesMap.clear();

    for (size_t i = 0; i < allCourses.size(); ++i) {
        const Course& course = allCourses[i];

        if (matchesSemesterFilter(course) && matchesSearchFilter(course, currentSearchText)) {
            filteredCourses.push_back(course);
            filteredIndicesMap.push_back(static_cast<int>(i));
        }
    }

    m_filteredCourseModel->populateCoursesData(filteredCourses, filteredIndicesMap);
}

bool CourseSelectionController::matchesSemesterFilter(const Course& course) const {
    if (currentSemesterFilter == "ALL") {
        return true;
    }

    if (currentSemesterFilter == "A") {
        return course.semester == 1 || course.semester == 4;
    }

    if (currentSemesterFilter == "B") {
        return course.semester == 2 || course.semester == 4;
    }

    if (currentSemesterFilter == "SUMMER") {
        return course.semester == 3 || course.semester == 4;
    }

    return true;
}

bool CourseSelectionController::matchesSearchFilter(const Course& course, const QString& searchText) const {
    if (searchText.isEmpty()) {
        return true;
    }

    QString searchLower = searchText.toLower();
    QString courseId = QString::fromStdString(course.raw_id).toLower();
    QString courseName = QString::fromStdString(course.name).toLower();
    QString teacherName = QString::fromStdString(course.teacher).toLower();

    return courseId.contains(searchLower) ||
           courseName.contains(searchLower) ||
           teacherName.contains(searchLower);
}
int CourseSelectionController::getSelectedCoursesCountForSemester(const QString& semester) {
    int count = 0;

    for (const auto& course : selectedCourses) {
        if (semester == "A" && (course.semester == 1 || course.semester == 4)) {
            count++;
        } else if (semester == "B" && (course.semester == 2 || course.semester == 4)) {
            count++;
        } else if (semester == "SUMMER" && (course.semester == 3 || course.semester == 4)) {
            count++;
        }
    }

    return count;
}

QVariantList CourseSelectionController::getSelectedCoursesForSemester(const QString& semester) {
    QVariantList result;

    for (int i = 0; i < static_cast<int>(selectedCourses.size()); i++) {
        const auto& course = selectedCourses[i];
        bool shouldInclude = false;

        if (semester == "A" && (course.semester == 1 || course.semester == 4)) {
            shouldInclude = true;
        } else if (semester == "B" && (course.semester == 2 || course.semester == 4)) {
            shouldInclude = true;
        } else if (semester == "SUMMER" && (course.semester == 3 || course.semester == 4)) {
            shouldInclude = true;
        }

        if (shouldInclude) {
            QVariantMap courseData;
            courseData["courseId"] = QString::fromStdString(course.raw_id);
            courseData["courseName"] = QString::fromStdString(course.name);
            courseData["originalIndex"] = i;
            result.append(courseData);
        }
    }

    return result;
}