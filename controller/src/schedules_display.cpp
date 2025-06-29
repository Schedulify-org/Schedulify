#include "schedules_display.h"
#include <algorithm>

SchedulesDisplayController::SchedulesDisplayController(QObject *parent)
        : ControllerManager(parent),
          m_scheduleModel(new ScheduleModel(this)),
          m_currentSemester("A"),
          m_allSemestersLoaded(false) {
    modelConnection = ModelAccess::getModel();

    // Initialize semester vectors
    m_schedulesA.clear();
    m_schedulesB.clear();
    m_schedulesSummer.clear();

    // Initialize loading states - all semesters start as not loading and not finished
    m_semesterLoadingState["A"] = false;
    m_semesterLoadingState["B"] = false;
    m_semesterLoadingState["SUMMER"] = false;

    m_semesterFinishedState["A"] = false;
    m_semesterFinishedState["B"] = false;
    m_semesterFinishedState["SUMMER"] = false;

    connect(this, &SchedulesDisplayController::schedulesSorted, this, [this]() {
        emit m_scheduleModel->scheduleDataChanged();
    });
}

SchedulesDisplayController::~SchedulesDisplayController() {
    modelConnection = nullptr;
}

// Old method for backward compatibility during transition
void SchedulesDisplayController::loadScheduleData(const std::vector<InformativeSchedule> &schedules) {
    // For backward compatibility, load into Semester A
    m_schedulesA = schedules;
    m_currentSemester = "A";
    m_scheduleModel->loadSchedules(m_schedulesA);
}

// Load schedules for a specific semester
void SchedulesDisplayController::loadSemesterScheduleData(const QString& semester, const std::vector<InformativeSchedule>& schedules) {
    if (semester == "A") {
        m_schedulesA = schedules;
        // If this is the first semester loaded, set it as current and update display
        if (m_currentSemester == "A") {
            m_scheduleModel->loadSchedules(m_schedulesA);
        }
    } else if (semester == "B") {
        m_schedulesB = schedules;
    } else if (semester == "SUMMER") {
        m_schedulesSummer = schedules;
    }

    // Mark semester as finished loading
    setSemesterFinished(semester, true);
    setSemesterLoading(semester, false);

    emit semesterSchedulesLoaded(semester);
}

// Switch between semesters
void SchedulesDisplayController::switchToSemester(const QString& semester) {
    if (m_currentSemester == semester) {
        return; // Already on this semester
    }

    // Check if the semester can be clicked (has finished loading)
    if (!canClickSemester(semester)) {
        qWarning() << "Semester" << semester << "is not ready to be clicked yet";
        return;
    }

    m_currentSemester = semester;

    // Load the appropriate schedules into the model
    if (semester == "A") {
        m_scheduleModel->loadSchedules(m_schedulesA);
    } else if (semester == "B") {
        m_scheduleModel->loadSchedules(m_schedulesB);
    } else if (semester == "SUMMER") {
        m_scheduleModel->loadSchedules(m_schedulesSummer);
    }

    emit currentSemesterChanged();
}

// Called when all semesters are generated
void SchedulesDisplayController::allSemestersGenerated() {
    m_allSemestersLoaded = true;
    emit allSemestersReady();
}

// Reset to Semester A
void SchedulesDisplayController::resetToSemesterA() {
    m_currentSemester = "A";
    // If Semester A has schedules, load them into the model
    if (!m_schedulesA.empty()) {
        m_scheduleModel->loadSchedules(m_schedulesA);
    }
    emit currentSemesterChanged();
}

// Check if a semester has schedules
bool SchedulesDisplayController::hasSchedulesForSemester(const QString& semester) const {
    if (semester == "A") {
        return !m_schedulesA.empty();
    } else if (semester == "B") {
        return !m_schedulesB.empty();
    } else if (semester == "SUMMER") {
        return !m_schedulesSummer.empty();
    }
    return false;
}

// Check if a semester is currently loading
bool SchedulesDisplayController::isSemesterLoading(const QString& semester) const {
    return m_semesterLoadingState.value(semester, false);
}

// Check if a semester has finished loading
bool SchedulesDisplayController::isSemesterFinished(const QString& semester) const {
    return m_semesterFinishedState.value(semester, false);
}

// Check if a semester button can be clicked
bool SchedulesDisplayController::canClickSemester(const QString& semester) const {
    // A semester can be clicked if:
    // 1. It has finished loading (has schedules)
    // 2. It's not currently loading
    return isSemesterFinished(semester) && !isSemesterLoading(semester) && hasSchedulesForSemester(semester);
}

// Set semester loading state
void SchedulesDisplayController::setSemesterLoading(const QString& semester, bool loading) {
    if (m_semesterLoadingState.value(semester, false) != loading) {
        m_semesterLoadingState[semester] = loading;
        emit semesterLoadingStateChanged(semester);
    }
}

// Set semester finished state
void SchedulesDisplayController::setSemesterFinished(const QString& semester, bool finished) {
    if (m_semesterFinishedState.value(semester, false) != finished) {
        m_semesterFinishedState[semester] = finished;
        emit semesterFinishedStateChanged(semester);
    }
}

// Get schedule count for a semester
int SchedulesDisplayController::getScheduleCountForSemester(const QString& semester) const {
    if (semester == "A") {
        return static_cast<int>(m_schedulesA.size());
    } else if (semester == "B") {
        return static_cast<int>(m_schedulesB.size());
    } else if (semester == "SUMMER") {
        return static_cast<int>(m_schedulesSummer.size());
    }
    return 0;
}

// Get current schedule vector
std::vector<InformativeSchedule>* SchedulesDisplayController::getCurrentScheduleVector() {
    if (m_currentSemester == "A") {
        return &m_schedulesA;
    } else if (m_currentSemester == "B") {
        return &m_schedulesB;
    } else if (m_currentSemester == "SUMMER") {
        return &m_schedulesSummer;
    }
    return nullptr;
}

// Apply sorting to current semester
void SchedulesDisplayController::applySorting(const QVariantMap& sortData) {
    QString sortField;
    bool isAscending = true;
    for (auto it = sortData.constBegin(); it != sortData.constEnd(); ++it) {
        const QVariantMap criterion = it.value().toMap();
        if (criterion.value("enabled").toBool()) {
            sortField = it.key();
            isAscending = criterion["ascending"].toBool();
            break;
        }
    }

    if (sortField.isEmpty()) {
        qWarning() << "No sorting field enabled!";
        clearSorting();
        return;
    }

    // Get reference to current semester's schedules
    std::vector<InformativeSchedule>* currentSchedules = getCurrentScheduleVector();
    if (!currentSchedules) {
        return;
    }

    // Apply sorting logic (same as before, but on current semester's schedules)
    if (sortField == m_currentSortField && isAscending != m_currentSortAscending) {
        std::reverse(currentSchedules->begin(), currentSchedules->end());
    } else {
        if (sortField == "amount_days") {
            std::vector<std::vector<InformativeSchedule>> buckets(8);
            for (const auto& sched : *currentSchedules) {
                if (sched.amount_days >= 1 && sched.amount_days <= 7)
                    buckets[sched.amount_days].push_back(sched);
                else
                    qWarning() << "amount_days out of range:" << sched.amount_days;
            }
            currentSchedules->clear();
            if (isAscending) {
                for (int i = 1; i <= 7; ++i)
                    currentSchedules->insert(currentSchedules->end(), buckets[i].begin(), buckets[i].end());
            } else {
                for (int i = 7; i >= 1; --i)
                    currentSchedules->insert(currentSchedules->end(), buckets[i].begin(), buckets[i].end());
            }
        } else if (sortField == "amount_gaps") {
            std::sort(currentSchedules->begin(), currentSchedules->end(), [isAscending](const InformativeSchedule& a, const InformativeSchedule& b) {
                return isAscending ? a.amount_gaps < b.amount_gaps : a.amount_gaps > b.amount_gaps;
            });
        } else if (sortField == "gaps_time") {
            std::sort(currentSchedules->begin(), currentSchedules->end(), [isAscending](const InformativeSchedule& a, const InformativeSchedule& b) {
                return isAscending ? a.gaps_time < b.gaps_time : a.gaps_time > b.gaps_time;
            });
        } else if (sortField == "avg_start") {
            std::sort(currentSchedules->begin(), currentSchedules->end(), [isAscending](const InformativeSchedule& a, const InformativeSchedule& b) {
                return isAscending ? a.avg_start < b.avg_start : a.avg_start > b.avg_start;
            });
        } else if (sortField == "avg_end") {
            std::sort(currentSchedules->begin(), currentSchedules->end(), [isAscending](const InformativeSchedule& a, const InformativeSchedule& b) {
                return isAscending ? a.avg_end < b.avg_end : a.avg_end > b.avg_end;
            });
        } else {
            qWarning() << "Unknown sorting key received:" << sortField;
            clearSorting();
            return;
        }
    }

    m_currentSortField = sortField;
    m_currentSortAscending = isAscending;
    m_scheduleModel->setCurrentScheduleIndex(0);
    m_scheduleModel->loadSchedules(*currentSchedules);
    emit schedulesSorted(static_cast<int>(currentSchedules->size()));
}

// Clear sorting for current semester
void SchedulesDisplayController::clearSorting() {
    std::vector<InformativeSchedule>* currentSchedules = getCurrentScheduleVector();
    if (!currentSchedules) {
        return;
    }

    // Reset to original order by index
    std::sort(currentSchedules->begin(), currentSchedules->end(), [](const InformativeSchedule& a, const InformativeSchedule& b) {
        return a.index < b.index;
    });

    m_currentSortField.clear();
    m_currentSortAscending = true;

    m_scheduleModel->loadSchedules(*currentSchedules);
    emit schedulesSorted(static_cast<int>(currentSchedules->size()));
}

// Save CSV for current semester
void SchedulesDisplayController::saveScheduleAsCSV() {
    std::vector<InformativeSchedule>* currentSchedules = getCurrentScheduleVector();
    if (!currentSchedules || currentSchedules->empty()) {
        return;
    }

    int currentIndex = m_scheduleModel->currentScheduleIndex();
    if (currentIndex >= 0 && currentIndex < static_cast<int>(currentSchedules->size())) {
        QString fileName = QFileDialog::getSaveFileName(nullptr,
                                                        "Save Schedule as CSV",
                                                        QDir::homePath() + "/" + generateFilename("",
                                                                                                  currentIndex + 1, fileType::CSV, m_currentSemester),
                                                        "CSV Files (*.csv)");
        if (!fileName.isEmpty()) {
            modelConnection->executeOperation(ModelOperation::SAVE_SCHEDULE,
                                              &(*currentSchedules)[currentIndex], fileName.toLocal8Bit().constData());
        }
    }
}

// Print schedule for current semester
void SchedulesDisplayController::printScheduleDirectly() {
    std::vector<InformativeSchedule>* currentSchedules = getCurrentScheduleVector();
    if (!currentSchedules || currentSchedules->empty()) {
        return;
    }

    int currentIndex = m_scheduleModel->currentScheduleIndex();
    if (currentIndex >= 0 && currentIndex < static_cast<int>(currentSchedules->size())) {
        modelConnection->executeOperation(ModelOperation::PRINT_SCHEDULE, &(*currentSchedules)[currentIndex], "");
    }
}

void SchedulesDisplayController::goBack() {
    emit navigateBack();
}

// Capture screenshot with semester in filename
void SchedulesDisplayController::captureAndSave(QQuickItem* item, const QString& savePath) {
    if (!item) {
        emit screenshotFailed();
        return;
    }

    QString path = savePath;
    if (path.isEmpty()) {
        int currentIndex = m_scheduleModel->currentScheduleIndex();
        path = QFileDialog::getSaveFileName(
                nullptr,
                tr("Save Screenshot"),
                generateFilename(QStandardPaths::writableLocation(QStandardPaths::PicturesLocation),
                                 currentIndex + 1, fileType::PNG, m_currentSemester),
                tr("Images (*.png)")
        );

        if (path.isEmpty()) {
            return;
        }
    } else {
        QFileInfo fileInfo(path);
        if (fileInfo.isDir()) {
            int currentIndex = m_scheduleModel->currentScheduleIndex();
            path = QDir(path).filePath(generateFilename("", currentIndex + 1, fileType::PNG, m_currentSemester));
        }
    }

    QSharedPointer<QQuickItemGrabResult> result = item->grabToImage();
    connect(result.data(), &QQuickItemGrabResult::ready, [this, result, path]() {
        if (result->saveToFile(path)) {
            emit screenshotSaved(path);
        } else {
            emit screenshotFailed();
        }
    });
}

// Generate filename with optional semester suffix
QString SchedulesDisplayController::generateFilename(const QString& basePath, int index, fileType type, const QString& semester) {
    QString filename;
    QString semesterSuffix = semester.isEmpty() ? "" : QString("_%1").arg(semester);

    switch (type) {
        case fileType::PNG:
            filename = QString("Schedule%1-%2.png").arg(semesterSuffix).arg(index);
            break;
        case fileType::CSV:
            filename = QString("Schedule%1-%2.csv").arg(semesterSuffix).arg(index);
            break;
    }

    if (basePath.isEmpty()) {
        return filename;
    }

    return QDir(basePath).filePath(filename);
}
void SchedulesDisplayController::clearAllSchedules() {
    // Clear all semester schedule vectors
    m_schedulesA.clear();
    m_schedulesB.clear();
    m_schedulesSummer.clear();

    // Reset all loading and finished states
    m_semesterLoadingState["A"] = false;
    m_semesterLoadingState["B"] = false;
    m_semesterLoadingState["SUMMER"] = false;

    m_semesterFinishedState["A"] = false;
    m_semesterFinishedState["B"] = false;
    m_semesterFinishedState["SUMMER"] = false;

    // Clear the current display
    m_scheduleModel->loadSchedules(std::vector<InformativeSchedule>());

    // Reset to semester A
    m_currentSemester = "A";
    m_allSemestersLoaded = false;

    // Emit signals to update UI
    emit currentSemesterChanged();
    emit semesterLoadingStateChanged("A");
    emit semesterLoadingStateChanged("B");
    emit semesterLoadingStateChanged("SUMMER");
    emit semesterFinishedStateChanged("A");
    emit semesterFinishedStateChanged("B");
    emit semesterFinishedStateChanged("SUMMER");
}