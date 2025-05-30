#include "schedules_display.h"

SchedulesDisplayController::SchedulesDisplayController(QObject *parent)
        : ControllerManager(parent),
          m_scheduleModel(new ScheduleModel(this)),
          m_scheduleFilter(new ScheduleFilter(this)) {
    modelConnection = ModelAccess::getModel();
    connect(this, &SchedulesDisplayController::filtersApplied,this, [this]() {
                emit m_scheduleModel->scheduleDataChanged();
    });
}

SchedulesDisplayController::~SchedulesDisplayController() {
    delete modelConnection;
    delete m_scheduleFilter;
}

void SchedulesDisplayController::loadScheduleData(const std::vector<InformativeSchedule> &schedules) {
    m_originalSchedules = schedules;
    m_filteredSchedules = schedules;
    m_scheduleModel->loadSchedules(m_filteredSchedules);
}

void SchedulesDisplayController::applyFilters(const QVariantMap& filterData) {
    ScheduleFilter::FilterCriteria criteria = convertQVariantToFilterCriteria(filterData);

    applyFiltersToSchedules(criteria);

    m_scheduleModel->loadSchedules(m_filteredSchedules);

    emit filtersApplied(static_cast<int>(m_filteredSchedules.size()),
                        static_cast<int>(m_originalSchedules.size()));
}

void SchedulesDisplayController::clearFilters() {
    m_filteredSchedules = m_originalSchedules;
    m_scheduleModel->loadSchedules(m_filteredSchedules);
    emit filtersApplied(static_cast<int>(m_filteredSchedules.size()),
                        static_cast<int>(m_originalSchedules.size()));
}

void SchedulesDisplayController::applyFiltersToSchedules(const ScheduleFilter::FilterCriteria& criteria) {
    m_filteredSchedules = m_scheduleFilter->filterSchedules(m_originalSchedules, criteria);
}

ScheduleFilter::FilterCriteria SchedulesDisplayController::convertQVariantToFilterCriteria(const QVariantMap& filterData) {
    ScheduleFilter::FilterCriteria criteria;

    // Days to Study Filter (serves as Active Days filter)
    if (filterData.contains("daysToStudy")) {
        QVariantMap daysToStudyData = filterData["daysToStudy"].toMap();
        criteria.daysToStudyEnabled = daysToStudyData["enabled"].toBool();
        if (criteria.daysToStudyEnabled) {
            criteria.daysToStudyValue = daysToStudyData["value"].toInt();
        }
    }

    // Total Gaps Filter
    if (filterData.contains("totalGaps")) {
        QVariantMap totalGapsData = filterData["totalGaps"].toMap();
        criteria.totalGapsEnabled = totalGapsData["enabled"].toBool();
        if (criteria.totalGapsEnabled) {
            criteria.totalGapsValue = totalGapsData["value"].toInt();
        }
    }

    // Max Gaps Time Filter
    if (filterData.contains("maxGapsTime")) {
        QVariantMap maxGapsTimeData = filterData["maxGapsTime"].toMap();
        criteria.maxGapsTimeEnabled = maxGapsTimeData["enabled"].toBool();
        if (criteria.maxGapsTimeEnabled) {
            criteria.maxGapsTimeValue = maxGapsTimeData["value"].toInt();
        }
    }

    // Average Day Start Filter
    if (filterData.contains("avgDayStart")) {
        QVariantMap avgDayStartData = filterData["avgDayStart"].toMap();
        criteria.avgDayStartEnabled = avgDayStartData["enabled"].toBool();
        if (criteria.avgDayStartEnabled) {
            criteria.avgDayStartHour = avgDayStartData["hour"].toInt();
            criteria.avgDayStartMinute = avgDayStartData["minute"].toInt();
        }
    }

    // Average Day End Filter
    if (filterData.contains("avgDayEnd")) {
        QVariantMap avgDayEndData = filterData["avgDayEnd"].toMap();
        criteria.avgDayEndEnabled = avgDayEndData["enabled"].toBool();
        if (criteria.avgDayEndEnabled) {
            criteria.avgDayEndHour = avgDayEndData["hour"].toInt();
            criteria.avgDayEndMinute = avgDayEndData["minute"].toInt();
        }
    }

    return criteria;
}

void SchedulesDisplayController::saveScheduleAsCSV() {
    int currentIndex = m_scheduleModel->currentScheduleIndex();
    if (currentIndex >= 0 && currentIndex < static_cast<int>(m_filteredSchedules.size())) {
        QString fileName = QFileDialog::getSaveFileName(nullptr,
                                                        "Save Schedule as CSV",
                                                        QDir::homePath() + "/" + generateFilename("",
                                                                                                  currentIndex + 1, fileType::CSV),
                                                        "CSV Files (*.csv)");
        if (!fileName.isEmpty()) {
            modelConnection->executeOperation(ModelOperation::SAVE_SCHEDULE,
                                              &m_filteredSchedules[currentIndex], fileName.toLocal8Bit().constData());
        }
    }
}

void SchedulesDisplayController::printScheduleDirectly() {
    int currentIndex = m_scheduleModel->currentScheduleIndex();
    if (currentIndex >= 0 && currentIndex < static_cast<int>(m_filteredSchedules.size())) {
        modelConnection->executeOperation(ModelOperation::PRINT_SCHEDULE, &m_filteredSchedules[currentIndex], "");
    }
}

void SchedulesDisplayController::goBack() {
    emit navigateBack();
}

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
                                 currentIndex + 1, fileType::PNG),
                tr("Images (*.png)")
        );

        if (path.isEmpty()) {
            return;
        }
    } else {
        QFileInfo fileInfo(path);
        if (fileInfo.isDir()) {
            int currentIndex = m_scheduleModel->currentScheduleIndex();
            path = QDir(path).filePath(generateFilename("", currentIndex + 1, fileType::PNG));
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

QString SchedulesDisplayController::generateFilename(const QString& basePath, int index, fileType type) {
    QString filename;
    switch (type) {
        case fileType::PNG:
            filename = QString("Schedule-%1.png").arg(index);
            break;
        case fileType::CSV:
            filename = QString("Schedule-%1.csv").arg(index);
            break;
    }

    if (basePath.isEmpty()) {
        return filename;
    }

    return QDir(basePath).filePath(filename);
}