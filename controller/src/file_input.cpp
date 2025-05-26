#include "file_input.h"

FileInputController::FileInputController(QObject *parent)
        : ControllerManager(parent) {
    modelConnection = ModelAccess::getModel();
}

FileInputController::~FileInputController() {
    delete modelConnection;
}

void FileInputController::handleUploadAndContinue() {
    // Open file dialog to select .txt file, starting in Documents
    QString fileName = QFileDialog::getOpenFileName(
            nullptr,
            "Select Course Input File",
            QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation),
            "Supported files (*.txt *.xlsx)"
    );

    if (fileName.isEmpty()) {
        Logger::get().logError("No file selected");
        return;
    }

    selectedFilePath = fileName;
    handleFileSelected(fileName);
}

void FileInputController::loadFile() {
    string filePath;

    if (!selectedFilePath.isEmpty()) {
        filePath = selectedFilePath.toLocal8Bit().constData();
    } else {
        Logger::get().logError("No file path available");
        emit invalidFileFormat();
        return;
    }

    auto* coursesPtr = static_cast<vector<Course>*>(
            modelConnection->executeOperation(ModelOperation::GENERATE_COURSES, nullptr, filePath)
    );

    if (!coursesPtr) {
        Logger::get().logError("could not find valid courses");
        emit invalidFileFormat();
        return;
    }

    vector<Course>& courses = *coursesPtr;

    auto* course_controller =
            qobject_cast<CourseSelectionController*>(findController("courseSelectionController"));

    if (course_controller && !courses.empty()) {
        course_controller->initiateCoursesData(courses);
        goToScreen(QUrl(QStringLiteral("qrc:/course_selection.qml")));
    } else {
        Logger::get().logError("Courses empty or controller missing");
        emit invalidFileFormat();
    }
}

void FileInputController::handleFileSelected(const QString &filePath) {

    if (!(filePath.endsWith(".txt", Qt::CaseInsensitive) || filePath.endsWith(".xlsx", Qt::CaseInsensitive))) {
        emit invalidFileFormat();
        Logger::get().logError("Invalid file type. Only .txt and .xlsx files are allowed");
        return;
    }

    if (filePath.isEmpty()) {
        emit fileSelected(false);
        emit errorMessage("No file was selected");
        Logger::get().logError("No file selected");
        return;
    } else {
        selectedFilePath = filePath;
        QString fileName = filePath.split('/').last().split('\\').last();
        emit fileNameChanged(fileName);
        emit fileSelected(true);
    }
}