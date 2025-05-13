
#include "schedules_display.h"

SchedulesDisplayController::SchedulesDisplayController(QObject *parent)
    : ControllerManager(parent), m_currentScheduleIndex(0) {
}

SchedulesDisplayController::~SchedulesDisplayController() = default;

void SchedulesDisplayController::loadScheduleData(const vector<InformativeSchedule> &schedules) {
    m_schedules = schedules;
    setCurrentScheduleIndex(0);
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

void SchedulesDisplayController::saveScheduleAsPDF() {
    if (m_currentScheduleIndex >= 0 && m_currentScheduleIndex < static_cast<int>(m_schedules.size())) {
        QString fileName = QFileDialog::getSaveFileName(nullptr,
                                                        "Save Schedule as PDF",
                                                        QDir::homePath() + "/schedule.csv",
                                                        "CSV Files (*.csv)");
        if (!fileName.isEmpty()) {
            // Call the PDF export function
            Model model;
            model.executeOperation(ModelOperation::SAVE_SCHEDULE, &m_schedules[m_currentScheduleIndex], fileName.toLocal8Bit().constData());
//            saveToPDF(m_schedules[m_currentScheduleIndex], fileName);
        }
    }
}

void SchedulesDisplayController::printScheduleDirectly() {
    if (m_currentScheduleIndex >= 0 && m_currentScheduleIndex < static_cast<int>(m_schedules.size())) {
        // Call the print function
        Model model;
        model.executeOperation(ModelOperation::PRINT_SCHEDULE, &m_schedules[m_currentScheduleIndex]);
//        printSchedule(m_schedules[m_currentScheduleIndex]);
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

    // Let user select path if not provided
    QString path = savePath;
    if (path.isEmpty()) {
        path = QFileDialog::getSaveFileName(
                nullptr,
                tr("Save Screenshot"),
                generateFilename(QStandardPaths::writableLocation(QStandardPaths::PicturesLocation), m_currentScheduleIndex+1),
                tr("Images (*.png)")
        );

        if (path.isEmpty()) {
            // User canceled the dialog
            return;
        }
    } else {
        // If path is just a directory, append generated filename
        QFileInfo fileInfo(path);
        if (fileInfo.isDir()) {
            path = QDir(path).filePath(generateFilename("", m_currentScheduleIndex+1));
        }
    }

    // Capture the screenshot
    QSharedPointer<QQuickItemGrabResult> result = item->grabToImage();
    connect(result.data(), &QQuickItemGrabResult::ready, [this, result, path]() {
        if (result->saveToFile(path)) {
            emit screenshotSaved(path);
        } else {
            emit screenshotFailed();
        }
    });
}

QString SchedulesDisplayController::generateFilename(const QString& basePath, int index) {
    QString filename = QString("Schedule-%1.png").arg(index);

    if (basePath.isEmpty()) {
        return filename;
    }

    return QDir(basePath).filePath(filename);
}