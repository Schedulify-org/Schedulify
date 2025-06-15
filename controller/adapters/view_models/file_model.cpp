#include "file_model.h"
#include "logger.h"
#include <algorithm>

FileHistoryModel::FileHistoryModel(QObject *parent)
        : QAbstractListModel(parent) {
    Logger::get().logInfo("FileHistoryModel created");
}

int FileHistoryModel::rowCount(const QModelIndex &parent) const {
    Q_UNUSED(parent)
    return static_cast<int>(m_files.size());
}

QVariant FileHistoryModel::data(const QModelIndex &index, int role) const {
    if (!index.isValid() || index.row() >= static_cast<int>(m_files.size())) {
        Logger::get().logWarning("Invalid index requested: " + std::to_string(index.row()) +
                                 " (max: " + std::to_string(m_files.size() - 1) + ")");
        return QVariant();
    }

    const FileEntity& file = m_files[index.row()];

    switch (role) {
        case IdRole:
            return file.id;
        case FileNameRole:
            return QString::fromStdString(file.file_name);
        case FileTypeRole:
            return QString::fromStdString(file.file_type).toUpper();
        case UploadTimeRole:
            return file.upload_time;
        case UpdatedAtRole:
            return file.updated_at;
        case FormattedDateRole:
            return formatDateTime(file.upload_time);
        default:
            return QVariant();
    }
}

QHash<int, QByteArray> FileHistoryModel::roleNames() const {
    QHash<int, QByteArray> roles;
    roles[IdRole] = "fileId";
    roles[FileNameRole] = "fileName";
    roles[FileTypeRole] = "fileType";
    roles[UploadTimeRole] = "uploadTime";
    roles[UpdatedAtRole] = "updatedAt";
    roles[FormattedDateRole] = "formattedDate";
    return roles;
}

void FileHistoryModel::populateFiles(const vector<FileEntity>& files) {
    Logger::get().logInfo("Populating file model with " + std::to_string(files.size()) + " files");

    beginResetModel();
    m_files = files;

    // Sort by upload_time in descending order (newest first)
    std::sort(m_files.begin(), m_files.end(), [](const FileEntity& a, const FileEntity& b) {
        return a.upload_time > b.upload_time;
    });

    endResetModel();

    // Log file details for debugging
    Logger::get().logInfo("File model populated - files now available:");
    for (size_t i = 0; i < m_files.size(); ++i) {
        const auto& file = m_files[i];
        Logger::get().logInfo("  [" + std::to_string(i) + "] ID=" + std::to_string(file.id) +
                              ", Name='" + file.file_name +
                              "', Type=" + file.file_type +
                              ", Uploaded=" + file.upload_time.toString("yyyy-MM-dd hh:mm:ss").toStdString());
    }
}

void FileHistoryModel::clearFiles() {
    Logger::get().logInfo("Clearing file model");
    beginResetModel();
    m_files.clear();
    endResetModel();
}

void FileHistoryModel::forceRefresh() {
    Logger::get().logInfo("Forcing file model refresh");
    beginResetModel();
    endResetModel();
}

int FileHistoryModel::getFileId(int index) const {
    if (!validateIndex(index)) {
        Logger::get().logError("Cannot get file ID for invalid index: " + std::to_string(index));
        return -1;
    }

    int fileId = m_files[index].id;
    Logger::get().logInfo("File ID for index " + std::to_string(index) + ": " + std::to_string(fileId));
    return fileId;
}

bool FileHistoryModel::validateIndex(int index) const {
    bool valid = (index >= 0 && index < static_cast<int>(m_files.size()));
    if (!valid) {
        Logger::get().logError("Invalid index: " + std::to_string(index) +
                               " (valid range: 0-" + std::to_string(m_files.size() - 1) + ")");
    }
    return valid;
}

void FileHistoryModel::logModelState() const {
    Logger::get().logInfo("=== FILE MODEL STATE ===");
    Logger::get().logInfo("Total files: " + std::to_string(m_files.size()));

    for (size_t i = 0; i < m_files.size(); ++i) {
        const auto& file = m_files[i];
        Logger::get().logInfo("File[" + std::to_string(i) + "]: ID=" + std::to_string(file.id) +
                              ", Name='" + file.file_name + "', Type=" + file.file_type);
    }
}

QString FileHistoryModel::formatDateTime(const QDateTime& dateTime) const {
    if (!dateTime.isValid()) {
        return "Unknown";
    }

    QDateTime now = QDateTime::currentDateTime();
    qint64 secondsAgo = dateTime.secsTo(now);

    if (secondsAgo < 60) {
        return "Just now";
    } else if (secondsAgo < 3600) {
        int minutes = secondsAgo / 60;
        return QString("%1 minute%2 ago").arg(minutes).arg(minutes > 1 ? "s" : "");
    } else if (secondsAgo < 86400) {
        int hours = secondsAgo / 3600;
        return QString("%1 hour%2 ago").arg(hours).arg(hours > 1 ? "s" : "");
    } else if (secondsAgo < 604800) {
        int days = secondsAgo / 86400;
        return QString("%1 day%2 ago").arg(days).arg(days > 1 ? "s" : "");
    } else {
        return dateTime.toString("MMM dd, yyyy hh:mm");
    }
}