#include "schedules_display.h"
#include <algorithm>
#include <vector>
#include <string>
#include <QDebug>

SchedulesDisplayController::SchedulesDisplayController(QObject *parent)
        : ControllerManager(parent),
          m_scheduleModel(new ScheduleModel(this)) {
    modelConnection = ModelAccess::getModel();

    // Initialize sort key mapping for better organization
    m_sortKeyMap["amount_days"] = "Days";
    m_sortKeyMap["amount_gaps"] = "Gaps";
    m_sortKeyMap["gaps_time"] = "Gap Time";
    m_sortKeyMap["avg_start"] = "Average Start";
    m_sortKeyMap["avg_end"] = "Average End";

    // Connect to schedule model filter state changes
    connect(m_scheduleModel, &ScheduleModel::filterStateChanged,
            this, &SchedulesDisplayController::onScheduleFilterStateChanged);

    connect(this, &SchedulesDisplayController::schedulesSorted, this, [this]() {
        emit m_scheduleModel->scheduleDataChanged();
    });
}

SchedulesDisplayController::~SchedulesDisplayController() {
    modelConnection = nullptr;
}

void SchedulesDisplayController::loadScheduleData(const std::vector<InformativeSchedule> &schedules) {
    m_schedules = schedules;
    m_scheduleModel->loadSchedules(m_schedules);
}

void SchedulesDisplayController::processBotMessage(const QString& userMessage) {
    if (!modelConnection) {
        emit botResponseReceived("I'm sorry, but I'm unable to process your request right now. Please try again later.");
        return;
    }

    qDebug() << "Processing bot message:" << userMessage;

    // Create bot query request with current available schedule IDs
    BotQueryRequest queryRequest = createBotQueryRequest(userMessage);

    // Start processing in a separate thread using new constructor
    QThread* workerThread = new QThread;
    BotWorker* worker = new BotWorker(modelConnection, queryRequest);
    worker->moveToThread(workerThread);

    // Connect signals
    connect(workerThread, &QThread::started, worker, &BotWorker::processMessage);

    // Use the new structured response signal
    connect(worker, QOverload<const BotQueryResponse&>::of(&BotWorker::responseReady),
            this, [this](const BotQueryResponse& response) {
                handleSimpleBotResponse(response);
            });

    connect(worker, &BotWorker::errorOccurred, this, [this](const QString& error) {
        emit botResponseReceived(error);
    });

    connect(worker, &BotWorker::finished, [worker, workerThread]() {
        worker->deleteLater();
        workerThread->quit();
        workerThread->deleteLater();
    });

    workerThread->start();
}

void SchedulesDisplayController::handleSimpleBotResponse(const BotQueryResponse& response) {
    if (response.hasError) {
        emit botResponseReceived(response.errorMessage.empty()
                                 ? QString("An error occurred while processing your request.")
                                 : QString::fromStdString(response.errorMessage));
        return;
    }

    // Always send the user message first
    emit botResponseReceived(QString::fromStdString(response.userMessage));

    // If this was a filter query, get the filtered IDs and apply them
    if (response.isFilterQuery) {
        qDebug() << "Filter query detected, getting filtered schedule IDs";

        // Get the filtered schedule IDs from the model
        void* result = modelConnection->executeOperation(ModelOperation::GET_LAST_FILTERED_IDS, nullptr, "");

        if (result) {
            auto* filteredIds = static_cast<std::vector<int>*>(result);

            if (filteredIds->empty()) {
                emit botResponseReceived("No schedules match your criteria. Try adjusting your requirements.");
            } else {
                // Convert to QVariantList and apply filter
                QVariantList qmlIds;
                for (int id : *filteredIds) {
                    qmlIds.append(id);
                }

                if (m_scheduleModel) {
                    m_scheduleModel->applyScheduleFilter(qmlIds);
                    qDebug() << "Applied filter with" << filteredIds->size() << "matching schedules";
                }
            }

            delete filteredIds;
        } else {
            emit botResponseReceived("Failed to apply schedule filter. Please try again.");
        }
    }
}

void SchedulesDisplayController::resetFilters() {
    if (m_scheduleModel && m_scheduleModel->isFiltered()) {
        m_scheduleModel->clearScheduleFilter();
        qDebug() << "Filters reset - showing all schedules";
    }
}

void SchedulesDisplayController::onScheduleFilterStateChanged() {
    emit filterStateChanged();

    if (m_scheduleModel) {
        int filteredCount = m_scheduleModel->scheduleCount();
        int totalCount = m_scheduleModel->totalScheduleCount();

        if (m_scheduleModel->isFiltered()) {
            emit schedulesFiltered(filteredCount, totalCount);
            qDebug() << "Schedule filter applied:" << filteredCount << "of" << totalCount << "schedules visible";
        } else {
            qDebug() << "Schedule filter cleared - all" << totalCount << "schedules visible";
        }
    }
}

BotQueryRequest SchedulesDisplayController::createBotQueryRequest(const QString& userMessage) {
    BotQueryRequest request;
    request.userMessage = userMessage.toStdString();
    request.scheduleMetadata = generateScheduleMetadata().toStdString();

    // Get available schedule IDs (currently visible ones)
    if (m_scheduleModel) {
        QVariantList currentIds = m_scheduleModel->isFiltered()
                                  ? m_scheduleModel->filteredScheduleIds()
                                  : m_scheduleModel->getAllScheduleIds();

        for (const QVariant& id : currentIds) {
            request.availableScheduleIds.push_back(id.toInt());
        }
    }

    qDebug() << "Created bot query request with" << request.availableScheduleIds.size() << "available schedules";
    return request;
}

QString SchedulesDisplayController::generateScheduleMetadata() {
    if (!m_scheduleModel || m_schedules.empty()) {
        return "No schedules available.";
    }

    QString metadata;
    metadata += "Schedule Database Structure:\n";
    metadata += "- Total schedules: " + QString::number(m_schedules.size()) + "\n";
    metadata += "- Currently filtered: " + QString(m_scheduleModel->isFiltered() ? "Yes" : "No") + "\n";
    metadata += "- Visible schedules: " + QString::number(m_scheduleModel->scheduleCount()) + "\n\n";

    metadata += "Available schedule fields for filtering:\n";
    metadata += "- index: Schedule ID (unique identifier)\n";
    metadata += "- amount_days: Number of study days (1-7)\n";
    metadata += "- amount_gaps: Total number of gaps between classes\n";
    metadata += "- gaps_time: Total time of gaps in minutes\n";
    metadata += "- avg_start: Average daily start time in minutes from midnight\n";
    metadata += "- avg_end: Average daily end time in minutes from midnight\n\n";

    // Add sample data for context
    metadata += "Sample schedule data (first few schedules):\n";
    int sampleCount = std::min(3, static_cast<int>(m_schedules.size()));
    for (int i = 0; i < sampleCount; ++i) {
        const auto& schedule = m_schedules[i];
        metadata += QString("Schedule %1: days=%2, gaps=%3, gap_time=%4min, avg_start=%5min, avg_end=%6min\n")
                .arg(schedule.index)
                .arg(schedule.amount_days)
                .arg(schedule.amount_gaps)
                .arg(schedule.gaps_time)
                .arg(schedule.avg_start)
                .arg(schedule.avg_end);
    }

    return metadata;
}

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

    // Check if we can make it in O(n)
    if (sortField == m_currentSortField && isAscending != m_currentSortAscending) {
        std::reverse(m_schedules.begin(), m_schedules.end());
    }
    else {
        if (sortField == "amount_days") {
            std::vector<std::vector<InformativeSchedule>> buckets(8); // Constant days
            for (const auto& sched : m_schedules) {
                if (sched.amount_days >= 1 && sched.amount_days <= 7)
                    buckets[sched.amount_days].push_back(sched);
                else
                    qWarning() << "amount_days out of range:" << sched.amount_days;
            }
            m_schedules.clear();
            if (isAscending) {
                for (int i = 1; i <= 7; ++i)
                    m_schedules.insert(m_schedules.end(), buckets[i].begin(), buckets[i].end());
            } else {
                for (int i = 7; i >= 1; --i)
                    m_schedules.insert(m_schedules.end(), buckets[i].begin(), buckets[i].end());
            }
        }
        else if (sortField == "amount_gaps") {
            std::sort(m_schedules.begin(), m_schedules.end(), [isAscending](const InformativeSchedule& a, const InformativeSchedule& b) {
                return isAscending ? a.amount_gaps < b.amount_gaps : a.amount_gaps > b.amount_gaps;
            });
        }
        else if (sortField == "gaps_time") {
            std::sort(m_schedules.begin(), m_schedules.end(), [isAscending](const InformativeSchedule& a, const InformativeSchedule& b) {
                return isAscending ? a.gaps_time < b.gaps_time : a.gaps_time > b.gaps_time;
            });
        }
        else if (sortField == "avg_start") {
            std::sort(m_schedules.begin(), m_schedules.end(), [isAscending](const InformativeSchedule& a, const InformativeSchedule& b) {
                return isAscending ? a.avg_start < b.avg_start : a.avg_start > b.avg_start;
            });
        }
        else if (sortField == "avg_end") {
            std::sort(m_schedules.begin(), m_schedules.end(), [isAscending](const InformativeSchedule& a, const InformativeSchedule& b) {
                return isAscending ? a.avg_end < b.avg_end : a.avg_end > b.avg_end;
            });
        }
        else {
            qWarning() << "Unknown sorting key received:" << sortField;
            clearSorting();
            return;
        }
    }

    m_currentSortField = sortField;
    m_currentSortAscending = isAscending;
    m_scheduleModel->setCurrentScheduleIndex(0);
    m_scheduleModel->loadSchedules(m_schedules);
    emit schedulesSorted(static_cast<int>(m_schedules.size()));
}

void SchedulesDisplayController::clearSorting() {
    // Reset to original order by index
    std::sort(m_schedules.begin(), m_schedules.end(), [](const InformativeSchedule& a, const InformativeSchedule& b) {
        return a.index < b.index;
    });

    m_currentSortField.clear();
    m_currentSortAscending = true;

    m_scheduleModel->loadSchedules(m_schedules);
    emit schedulesSorted(static_cast<int>(m_schedules.size()));
}

void SchedulesDisplayController::saveScheduleAsCSV() {
    int currentIndex = m_scheduleModel->currentScheduleIndex();
    const auto& activeSchedules = m_scheduleModel->getCurrentSchedules();

    if (currentIndex >= 0 && currentIndex < static_cast<int>(activeSchedules.size())) {
        QString fileName = QFileDialog::getSaveFileName(nullptr,
                                                        "Save Schedule as CSV",
                                                        QDir::homePath() + "/" + generateFilename("",
                                                                                                  currentIndex + 1, fileType::CSV),
                                                        "CSV Files (*.csv)");
        if (!fileName.isEmpty()) {
            modelConnection->executeOperation(ModelOperation::SAVE_SCHEDULE,
                                              &activeSchedules[currentIndex], fileName.toLocal8Bit().constData());
        }
    }
}

void SchedulesDisplayController::printScheduleDirectly() {
    int currentIndex = m_scheduleModel->currentScheduleIndex();
    const auto& activeSchedules = m_scheduleModel->getCurrentSchedules();

    if (currentIndex >= 0 && currentIndex < static_cast<int>(activeSchedules.size())) {
        modelConnection->executeOperation(ModelOperation::PRINT_SCHEDULE, &activeSchedules[currentIndex], "");
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