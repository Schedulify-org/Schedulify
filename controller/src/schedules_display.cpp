
#include "schedules_display.h"

SchedulesDisplayController::SchedulesDisplayController(QObject *parent)
    : ControllerManager(parent), m_currentScheduleIndex(0) {
    modelConnection = ModelAccess::getModel();
}

SchedulesDisplayController::~SchedulesDisplayController() {
    // delete modelConnection;
};

void SchedulesDisplayController::loadScheduleData(
        const std::vector<InformativeSchedule>& schedules)
{
    qDebug() << "[Schedulify] Loaded" << schedules.size() << "schedules";
    m_allSchedules = schedules;
    m_schedules = m_allSchedules;

    m_filteredIndices.resize(schedules.size());
    std::iota(m_filteredIndices.begin(), m_filteredIndices.end(), 0);
    setCurrentScheduleIndex(0); // This will now correctly set the index as m_schedules is populated
    emit scheduleCountChanged();
}

int SchedulesDisplayController::currentScheduleIndex() const {
    return m_currentScheduleIndex;
}

void SchedulesDisplayController::setCurrentScheduleIndex(int index) {
    if (index >= 0 && index < static_cast<int>(m_schedules.size()) && m_currentScheduleIndex != index) {
        m_currentScheduleIndex = index;
        emit currentScheduleIndexChanged();
        emit scheduleChanged();
    }
}

QVariantList SchedulesDisplayController::getDayItems(int scheduleIndex, int dayIndex) const {
    if (scheduleIndex < 0 || scheduleIndex >= static_cast<int>(m_schedules.size()))
        return {};

    if (dayIndex < 0 || dayIndex >= static_cast<int>(m_schedules[scheduleIndex].week.size()))
        return {};

    QVariantList items;
    for (const auto &item : m_schedules[scheduleIndex].week[dayIndex].day_items) {
        QVariantMap itemMap;
        itemMap["courseName"] = QString::fromStdString(item.courseName);
        itemMap["raw_id"] = QString::fromStdString(item.raw_id);
        itemMap["type"] = QString::fromStdString(item.type);
        itemMap["start"] = QString::fromStdString(item.start);
        itemMap["end"] = QString::fromStdString(item.end);
        itemMap["building"] = QString::fromStdString(item.building);
        itemMap["room"] = QString::fromStdString(item.room);
        items.append(itemMap);
    }
    return items;
}

QString SchedulesDisplayController::getDayName(int dayIndex) {
    const QString dayNames[] = {
        "ראשון", "שני", "שלישי", "רביעי", "חמישי", "שישי", "שבת"
    };

    if (dayIndex >= 0 && dayIndex < 7)
        return dayNames[dayIndex];
    return "";
}

int SchedulesDisplayController::getScheduleCount() const {
    return static_cast<int>(m_schedules.size());
}

void SchedulesDisplayController::applyFilters(const QVariantMap& filters) {
    ScheduleFilters newFilters;

    if (filters.isEmpty()) {
        clearFilters();
        return;
    }

    if (filters.contains("totalGaps") && filters["totalGaps"].isValid()) {
        newFilters.maxGapsPerDay = filters["totalGaps"].toInt();
    }

    if (filters.contains("maxGapsTime") && filters["maxGapsTime"].isValid()) {
        newFilters.minBreakMinutes = filters["maxGapsTime"].toInt();
    }

    if (filters.contains("daysToStudy") && filters["daysToStudy"].isValid()) {
        newFilters.maxDaysToStudy = filters["daysToStudy"].toInt();
    }

    m_currentFilters = newFilters; // Update the member variable
    m_filtersActive = true;        // Indicate filters are active
    applyScheduleFilters();        // Apply the filters to the schedule list

    emit filtersChanged();         // Notify QML that filters have been applied
    emit scheduleCountChanged();   // Notify QML the count might have changed
    // currentScheduleIndexChanged() and scheduleChanged() are handled within applyScheduleFilters now
}

void SchedulesDisplayController::clearFilters() {
    m_filtersActive = false;
    m_schedules = m_allSchedules;
    m_filteredIndices.clear();

    for (size_t i = 0; i < m_allSchedules.size(); ++i) {
        m_filteredIndices.push_back(i);
    }

    setCurrentScheduleIndex(0);
    emit filtersChanged();
    emit scheduleCountChanged();
    emit currentScheduleIndexChanged();
    emit scheduleChanged();
}

bool SchedulesDisplayController::hasFilters() const {
    return m_filtersActive;
}

void SchedulesDisplayController::applyScheduleFilters() {
    if (!m_filtersActive) return;

    m_schedules.clear();
    m_filteredIndices.clear();

    for (size_t i = 0; i < m_allSchedules.size(); ++i) {
        const InformativeSchedule& schedule = m_allSchedules[i];
        bool passes = true;

        if (m_currentFilters.maxDaysToStudy < 7 && m_currentFilters.maxDaysToStudy > 0) {
            passes &= passesMaxDaysFilter(schedule, m_currentFilters.maxDaysToStudy);
        }

        // if (m_currentFilters.maxGapsPerDay >= 0) {
        //     passes &= passesGapFilter(schedule, m_currentFilters.maxGapsPerDay, /*minBreak=*/0);
        // }
        //
        // if (m_currentFilters.avgDayStartHour >= 0) {
        //     int targetMinutes = m_currentFilters.avgDayStartHour * 60 + m_currentFilters.avgDayStartMinute;
        //     passes &= avgDayStart(schedule) <= targetMinutes;
        // }
        //
        // if (m_currentFilters.avgDayEndHour >= 0) {
        //     int targetMinutes = m_currentFilters.avgDayEndHour * 60 + m_currentFilters.avgDayEndMinute;
        //     passes &= avgDayEnd(schedule) >= targetMinutes;
        // }

        if (passes) {
            m_schedules.push_back(schedule);
            m_filteredIndices.push_back(i);
        }
    }

    // Crucially, ensure the current index is valid after filtering
    // If no schedules pass the filter, set to -1 or handle appropriately
    if (m_schedules.empty()) {
        setCurrentScheduleIndex(-1); // Or 0 if you want to show "No schedules"
    } else if (m_currentScheduleIndex >= static_cast<int>(m_schedules.size())) {
        setCurrentScheduleIndex(0); // Reset to the first schedule if current is out of bounds
    } else if (m_currentScheduleIndex < 0 && !m_schedules.empty()) {
        setCurrentScheduleIndex(0); // If it was -1 and schedules exist, set to 0
    }
    // else, if m_currentScheduleIndex is valid and within bounds, keep it as is.

    // Always emit signals to notify QML that the schedule list and count might have changed.
    emit scheduleCountChanged();     // Notify QML the total count has changed
    emit currentScheduleIndexChanged(); // Notify QML if the current index might have been reset
    emit scheduleChanged();          // Notify QML that the current schedule's data might have changed
}


double SchedulesDisplayController::avgDayStart(const InformativeSchedule& schedule) {
    int totalMinutes = 0;
    int dayCount = 0;

    for (const auto& day : schedule.week) {
        if (day.day_items.empty()) continue;

        int earliestStart = INT_MAX;
        for (const auto& item : day.day_items) {
            QStringList parts = QString::fromStdString(item.start).split(":");
            if (parts.size() < 2) continue;
            int minutes = parts[0].toInt() * 60 + parts[1].toInt();
            if (minutes < earliestStart)
                earliestStart = minutes;
        }

        if (earliestStart != INT_MAX) {
            totalMinutes += earliestStart;
            ++dayCount;
        }
    }

    return (dayCount == 0) ? 0.0 : static_cast<double>(totalMinutes) / dayCount;
}

double SchedulesDisplayController::avgDayEnd(const InformativeSchedule& schedule) {
    int totalMinutes = 0;
    int dayCount = 0;

    for (const auto& day : schedule.week) {
        if (day.day_items.empty()) continue;

        int latestEnd = INT_MIN;
        for (const auto& item : day.day_items) {
            QStringList parts = QString::fromStdString(item.end).split(":");
            if (parts.size() < 2) continue;
            int minutes = parts[0].toInt() * 60 + parts[1].toInt();
            if (minutes > latestEnd)
                latestEnd = minutes;
        }

        if (latestEnd != INT_MIN) {
            totalMinutes += latestEnd;
            ++dayCount;
        }
    }

    return (dayCount == 0) ? 0.0 : static_cast<double>(totalMinutes) / dayCount;
}

bool SchedulesDisplayController::passesGapFilter(const InformativeSchedule& schedule, int maxGaps, int minBreak) const {
    for (const auto& day : schedule.week) {
        if (day.day_items.empty()) continue;

        // Check gaps in this day
        if (maxGaps >= 0) {
            int gaps = calculateGapsInDay(day.day_items);
            if (gaps > maxGaps) {
                return false;
            }
        }

        // Check minimum break between classes
        if (minBreak > 0) {
            std::vector<std::pair<int, int>> timeRanges;

            for (const auto& item : day.day_items) {
                QStringList startParts = QString::fromStdString(item.start).split(":");
                QStringList endParts = QString::fromStdString(item.end).split(":");

                if (startParts.size() >= 2 && endParts.size() >= 2) {
                    int start = startParts[0].toInt() * 60 + startParts[1].toInt();
                    int end = endParts[0].toInt() * 60 + endParts[1].toInt();
                    timeRanges.push_back({start, end});
                }
            }

            // Sort by start time
            std::sort(timeRanges.begin(), timeRanges.end());

            // Check breaks between consecutive classes
            for (size_t i = 1; i < timeRanges.size(); ++i) {
                int breakTime = timeRanges[i].first - timeRanges[i-1].second;
                if (breakTime > 0 && breakTime < minBreak) {
                    return false;
                }
            }
        }
    }
    return true;
}

bool SchedulesDisplayController::passesMaxDaysFilter(const InformativeSchedule& schedule, int maxDays) {
    int daysWithClasses = 0;
    for (const auto& day : schedule.week) {
        if (!day.day_items.empty()) {
            daysWithClasses++;
        }
    }
    return daysWithClasses <= maxDays;
}

int SchedulesDisplayController::calculateGapsInDay(const std::vector<ScheduleItem>& dayItems)
{
    if (dayItems.size() <= 1) return 0;

    std::vector<std::pair<int, int>> timeRanges;

    for (const auto& item : dayItems) {
        QStringList startParts = QString::fromStdString(item.start).split(":");
        QStringList endParts = QString::fromStdString(item.end).split(":");

        if (startParts.size() >= 2 && endParts.size() >= 2) {
            int start = startParts[0].toInt() * 60 + startParts[1].toInt();
            int end = endParts[0].toInt() * 60 + endParts[1].toInt();
            timeRanges.push_back({start, end});
        }
    }

    // Sort by start time
    std::sort(timeRanges.begin(), timeRanges.end());

    int gaps = 0;
    for (size_t i = 1; i < timeRanges.size(); ++i) {
        if (timeRanges[i].first > timeRanges[i-1].second) {
            gaps++;
        }
    }

    return gaps;
}

void SchedulesDisplayController::saveScheduleAsCSV() {
    if (m_currentScheduleIndex >= 0 && m_currentScheduleIndex < static_cast<int>(m_schedules.size())) {
        QString fileName = QFileDialog::getSaveFileName(nullptr,
                                                        "Save Schedule as PDF",
                                                        QDir::homePath() + "/" + generateFilename("",
                                                              m_currentScheduleIndex+1, fileType::CSV),
                                                        "CSV Files (*.csv)");
        if (!fileName.isEmpty()) {
            modelConnection->executeOperation(ModelOperation::SAVE_SCHEDULE,
                                  &m_schedules[m_currentScheduleIndex], fileName.toLocal8Bit().constData());
        }
    }
}

void SchedulesDisplayController::printScheduleDirectly() {
    if (m_currentScheduleIndex >= 0 && m_currentScheduleIndex < static_cast<int>(m_schedules.size())) {
        modelConnection->executeOperation(ModelOperation::PRINT_SCHEDULE, &m_schedules[m_currentScheduleIndex], "");
    }
}

void SchedulesDisplayController::goBack() {
    qDebug() << "[Schedulify] goBack";
    emit navigateBack();
}

void SchedulesDisplayController::captureAndSave(QQuickItem* item, const QString& savePath) {
    if (!item) {
        emit screenshotFailed();
        return;
    }

    QString path = savePath;
    if (path.isEmpty()) {
        path = QFileDialog::getSaveFileName(
                nullptr,
                tr("Save Screenshot"),
                generateFilename(QStandardPaths::writableLocation(QStandardPaths::PicturesLocation),
                                 m_currentScheduleIndex+1, fileType::PNG),
                tr("Images (*.png)")
        );

        if (path.isEmpty()) {
            // User canceled the dialog
            return;
        }
    } else {
        QFileInfo fileInfo(path);
        if (fileInfo.isDir()) {
            path = QDir(path).filePath(generateFilename("", m_currentScheduleIndex+1
                                                        , fileType::PNG));
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