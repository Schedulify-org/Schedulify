#include "file_input.h"

FileInputController::FileInputController(QObject *parent)
        : ControllerManager(parent)
        , m_fileHistoryModel(new FileHistoryModel(this)) {
    modelConnection = ModelAccess::getModel();

    // Load file history on initialization
    loadFileHistory();

    Logger::get().logInfo("FileInputController initialized with " +
                          std::to_string(m_fileHistoryModel->rowCount()) + " files in history");
}

FileInputController::~FileInputController() {
    modelConnection = nullptr;
}

void FileInputController::loadFileHistory() {
    Logger::get().logInfo("Loading file history from database...");

    auto* fileHistoryPtr = static_cast<vector<FileEntity>*>(
            modelConnection->executeOperation(ModelOperation::GET_FILE_HISTORY, nullptr, "")
    );

    if (fileHistoryPtr) {
        Logger::get().logInfo("Retrieved " + std::to_string(fileHistoryPtr->size()) + " files from database");

        // Log file details for debugging
        for (const auto& file : *fileHistoryPtr) {
            Logger::get().logInfo("File: ID=" + std::to_string(file.id) +
                                  ", Name='" + file.file_name +
                                  "', Type='" + file.file_type +
                                  "', Uploaded=" + file.upload_time.toString("yyyy-MM-dd hh:mm:ss").toStdString());
        }

        m_fileHistoryModel->populateFiles(*fileHistoryPtr);
        delete fileHistoryPtr;

        // Clear any existing selections when reloading history
        m_selectedFileIds.clear();
        emit fileSelectionChanged();

        Logger::get().logInfo("File history model populated successfully");
    } else {
        Logger::get().logError("Failed to retrieve file history from database");
        m_fileHistoryModel->clearFiles();
    }
}

void FileInputController::refreshFileHistory() {
    Logger::get().logInfo("=== REFRESHING FILE HISTORY ===");
    loadFileHistory();
    Logger::get().logInfo("File history refreshed - " + std::to_string(m_fileHistoryModel->rowCount()) + " files available");
}

void FileInputController::deleteFileFromHistory(int fileId) {
    Logger::get().logInfo("=== DELETING FILE FROM HISTORY ===");
    Logger::get().logInfo("File ID to delete: " + std::to_string(fileId));

    if (fileId <= 0) {
        Logger::get().logError("Invalid file ID for deletion: " + std::to_string(fileId));
        emit errorMessage("Invalid file ID for deletion");
        return;
    }

    try {
        // Get file details before deletion for logging
        for (int i = 0; i < m_fileHistoryModel->rowCount(); ++i) {
            if (m_fileHistoryModel->getFileId(i) == fileId) {
                QModelIndex index = m_fileHistoryModel->index(i, 0);
                QString fileName = m_fileHistoryModel->data(index, FileHistoryModel::FileNameRole).toString();
                QString fileType = m_fileHistoryModel->data(index, FileHistoryModel::FileTypeRole).toString();

                Logger::get().logInfo("Deleting file: '" + fileName.toStdString() + "' (" + fileType.toStdString() + ")");
                break;
            }
        }

        // Call model to delete file and its courses
        bool* result = static_cast<bool*>(
                modelConnection->executeOperation(ModelOperation::DELETE_FILE_FROM_HISTORY, &fileId, "")
        );

        if (result && *result) {
            Logger::get().logInfo("Successfully deleted file and courses from database");

            // Remove from selection if it was selected
            auto it = std::find(m_selectedFileIds.begin(), m_selectedFileIds.end(), fileId);
            if (it != m_selectedFileIds.end()) {
                m_selectedFileIds.erase(it);
                emit fileSelectionChanged();
                Logger::get().logInfo("Removed deleted file from selection");
            }

            // Refresh the file history to update the UI
            refreshFileHistory();

            emit errorMessage("File deleted successfully from history");

            delete result;
        } else {
            Logger::get().logError("Failed to delete file from history");
            emit errorMessage("Failed to delete file from history. Please try again.");
            if (result) delete result;
        }

    } catch (const std::exception& e) {
        Logger::get().logError("Exception during file deletion: " + string(e.what()));
        emit errorMessage("Error occurred while deleting file: " + QString::fromStdString(e.what()));
    }
}

void FileInputController::handleUploadAndContinue() {
    // Open file dialog to select .txt or .xlsx file, starting in Documents
    QString fileName = QFileDialog::getOpenFileName(
            nullptr,
            "Select Course Input File",
            QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation),
            "Supported files (*.txt *.xlsx)"
    );

    if (fileName.isEmpty()) {
        Logger::get().logWarning("No file selected in file dialog");
        return;
    }

    selectedFilePath = fileName;
    handleFileSelected(fileName);
}

void FileInputController::loadNewFile() {
    if (selectedFilePath.isEmpty()) {
        Logger::get().logError("No file path available for loading");
        emit invalidFileFormat();
        return;
    }

    string filePath = selectedFilePath.toUtf8().constData();
    Logger::get().logInfo("=== LOADING NEW FILE ===");
    Logger::get().logInfo("File path: " + filePath);

    auto* coursesPtr = static_cast<vector<Course>*>(
            modelConnection->executeOperation(ModelOperation::GENERATE_COURSES, nullptr, filePath)
    );

    if (!coursesPtr) {
        Logger::get().logError("Failed to generate courses from file");
        emit invalidFileFormat();
        return;
    }

    vector<Course>& courses = *coursesPtr;

    if (courses.empty()) {
        Logger::get().logError("No courses found in file");
        emit invalidFileFormat();
        return;
    }

    Logger::get().logInfo("Successfully loaded " + std::to_string(courses.size()) + " courses from new file");

    // Refresh file history after loading new file (should show the newly uploaded file)
    refreshFileHistory();

    proceedWithCourses(courses);
}

void FileInputController::loadFromHistory() {
    if (m_selectedFileIds.empty()) {
        Logger::get().logError("No files selected from history");
        emit errorMessage("Please select at least one file from history");
        return;
    }

    Logger::get().logInfo("=== LOADING FROM HISTORY ===");
    Logger::get().logInfo("Loading from " + std::to_string(m_selectedFileIds.size()) + " selected file(s)");

    // Log the file IDs being loaded
    string fileIdsList;
    for (size_t i = 0; i < m_selectedFileIds.size(); ++i) {
        if (i > 0) fileIdsList += ", ";
        fileIdsList += std::to_string(m_selectedFileIds[i]);
    }
    Logger::get().logInfo("Selected file IDs: [" + fileIdsList + "]");

    // Log file details for the selected IDs
    for (int fileId : m_selectedFileIds) {
        for (int i = 0; i < m_fileHistoryModel->rowCount(); ++i) {
            if (m_fileHistoryModel->getFileId(i) == fileId) {
                QModelIndex index = m_fileHistoryModel->index(i, 0);
                QString fileName = m_fileHistoryModel->data(index, FileHistoryModel::FileNameRole).toString();
                QString fileType = m_fileHistoryModel->data(index, FileHistoryModel::FileTypeRole).toString();
                QString uploadTime = m_fileHistoryModel->data(index, FileHistoryModel::FormattedDateRole).toString();

                Logger::get().logInfo("Selected file: ID=" + std::to_string(fileId) +
                                      ", Name='" + fileName.toStdString() +
                                      "', Type=" + fileType.toStdString() +
                                      ", Uploaded=" + uploadTime.toStdString());
                break;
            }
        }
    }

    FileLoadData loadData;
    loadData.fileIds = m_selectedFileIds;
    loadData.operation_type = "load_from_history";

    auto* coursesPtr = static_cast<vector<Course>*>(
            modelConnection->executeOperation(ModelOperation::LOAD_FROM_HISTORY, &loadData, "")
    );

    if (!coursesPtr) {
        Logger::get().logError("Failed to load courses from history");
        emit errorMessage("Failed to load courses from selected files");
        return;
    }

    vector<Course>& courses = *coursesPtr;

    Logger::get().logInfo("=== HISTORY LOADING RESULT ===");
    Logger::get().logInfo("Loaded " + std::to_string(courses.size()) + " courses from history");

    if (courses.empty()) {
        Logger::get().logError("No courses found in selected files");
        emit errorMessage("No courses found in selected files. The files may be corrupted or have no associated course data.");
        return;
    }

    proceedWithCourses(courses);
}

void FileInputController::proceedWithCourses(const vector<Course>& courses) {
    auto* course_controller =
            qobject_cast<CourseSelectionController*>(findController("courseSelectionController"));

    if (course_controller) {
        Logger::get().logInfo("Proceeding to course selection with " + std::to_string(courses.size()) + " courses");
        course_controller->initiateCoursesData(courses);
        goToScreen(QUrl(QStringLiteral("qrc:/course_selection.qml")));
    } else {
        Logger::get().logError("Course selection controller not found");
        emit invalidFileFormat();
    }
}

void FileInputController::handleFileSelected(const QString &filePath) {
    if (!(filePath.endsWith(".txt", Qt::CaseInsensitive) || filePath.endsWith(".xlsx", Qt::CaseInsensitive))) {
        Logger::get().logError("Invalid file type: " + filePath.toStdString() + ". Only .txt and .xlsx files are allowed");
        emit invalidFileFormat();
        return;
    }

    if (filePath.isEmpty()) {
        emit fileSelected(false);
        emit errorMessage("No file was selected");
        Logger::get().logError("Empty file path provided");
        return;
    }

    selectedFilePath = filePath;

    QFileInfo fileInfo(filePath);
    QString fileName = fileInfo.fileName();

    Logger::get().logInfo("File selected: " + fileName.toStdString() + " (path: " + filePath.toStdString() + ")");

    emit fileNameChanged(fileName);
    emit fileSelected(true);
}

void FileInputController::toggleFileSelection(int index) {
    if (index < 0 || index >= m_fileHistoryModel->rowCount()) {
        Logger::get().logError("Invalid file index for selection: " + std::to_string(index) +
                               " (valid range: 0-" + std::to_string(m_fileHistoryModel->rowCount() - 1) + ")");
        return;
    }

    int fileId = m_fileHistoryModel->getFileId(index);
    if (fileId == -1) {
        Logger::get().logError("Could not get file ID for index: " + std::to_string(index));
        return;
    }

    // Get file details for logging
    QModelIndex modelIndex = m_fileHistoryModel->index(index, 0);
    QString fileName = m_fileHistoryModel->data(modelIndex, FileHistoryModel::FileNameRole).toString();

    auto it = std::find(m_selectedFileIds.begin(), m_selectedFileIds.end(), fileId);

    if (it != m_selectedFileIds.end()) {
        // File is selected, remove it
        m_selectedFileIds.erase(it);
        Logger::get().logInfo("Deselected file: '" + fileName.toStdString() + "' (ID: " + std::to_string(fileId) + ")");
    } else {
        // File is not selected, add it
        m_selectedFileIds.push_back(fileId);
        Logger::get().logInfo("Selected file: '" + fileName.toStdString() + "' (ID: " + std::to_string(fileId) + ")");
    }

    // Force model update by emitting dataChanged for this specific index
    emit m_fileHistoryModel->dataChanged(modelIndex, modelIndex);

    emit fileSelectionChanged();
    logFileSelectionState();
}

bool FileInputController::isFileSelected(int index) {
    if (index < 0 || index >= m_fileHistoryModel->rowCount()) {
        return false;
    }

    int fileId = m_fileHistoryModel->getFileId(index);
    if (fileId == -1) {
        return false;
    }

    bool selected = std::find(m_selectedFileIds.begin(), m_selectedFileIds.end(), fileId) != m_selectedFileIds.end();
    return selected;
}

void FileInputController::clearFileSelection() {
    if (!m_selectedFileIds.empty()) {
        Logger::get().logInfo("Clearing selection of " + std::to_string(m_selectedFileIds.size()) + " file(s)");
        m_selectedFileIds.clear();

        // Force a model refresh to update all checkboxes using the public method
        m_fileHistoryModel->forceRefresh();

        emit fileSelectionChanged();
        Logger::get().logInfo("All file selections cleared");
    }
}

int FileInputController::selectedFileCount() const {
    return static_cast<int>(m_selectedFileIds.size());
}

void FileInputController::logFileSelectionState() {
    Logger::get().logInfo("=== FILE SELECTION STATE ===");
    Logger::get().logInfo("Total files in history: " + std::to_string(m_fileHistoryModel->rowCount()));
    Logger::get().logInfo("Selected files: " + std::to_string(m_selectedFileIds.size()));

    if (!m_selectedFileIds.empty()) {
        string selectedList;
        for (size_t i = 0; i < m_selectedFileIds.size(); ++i) {
            if (i > 0) selectedList += ", ";
            selectedList += std::to_string(m_selectedFileIds[i]);
        }
        Logger::get().logInfo("Selected file IDs: [" + selectedList + "]");
    }
}

void FileInputController::validateFileSelection() {
    Logger::get().logInfo("=== VALIDATING FILE SELECTION ===");

    // Check if selected file IDs actually exist in the model
    for (int fileId : m_selectedFileIds) {
        bool found = false;
        for (int i = 0; i < m_fileHistoryModel->rowCount(); ++i) {
            if (m_fileHistoryModel->getFileId(i) == fileId) {
                found = true;
                break;
            }
        }

        if (!found) {
            Logger::get().logWarning("Selected file ID " + std::to_string(fileId) + " not found in current model");
        }
    }
}