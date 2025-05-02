#include "file_input.h"

FileInputController::FileInputController(QObject *parent)
        : ControllerManager(parent) {}

void FileInputController::handleUploadAndContinue() {
    // Open file dialog to select .txt file, starting in Documents
    QString fileName = QFileDialog::getOpenFileName(
            nullptr,
            "Select Course Input File",
            QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation),
            "Text Files (*.txt)"
    );

    if (fileName.isEmpty()) {
        qDebug() << "No file selected.";
        return;
    }

    selectedFilePath = fileName;
    handleFileSelected(fileName);
}

void FileInputController::loadFile() {
    Model model;
    string filePath;

    if (!selectedFilePath.isEmpty()) {
        filePath = selectedFilePath.toStdString();
    } else {
        qWarning() << "Failed to find courseSelectionController or courses are empty!";
        return;
    }

    //generate Courses vector
    auto* coursesPtr = static_cast<vector<Course>*>
    (model.executeOperation(ModelOperation::GENERATE_COURSES, nullptr, filePath));

    vector<Course>& courses = *coursesPtr;

    auto* course_controller =
            qobject_cast<CourseSelectionController*>(findController("courseSelectionController"));


    if (course_controller && !courses.empty()) {
        // Initialize the course data first
        course_controller->initiateCoursesData(courses);

        // Then navigate to course selection screen
        goToScreen(QUrl(QStringLiteral("qrc:/course_selection.qml")));
    } else {
        qWarning() << "Failed to find courseSelectionController or courses are empty!";
        return;
    }
}

void FileInputController::handleFileSelected(const QString &filePath) {
    selectedFilePath = filePath;

    // Emit signal that a file has been selected
    emit fileSelected(!filePath.isEmpty());

    // Extract just the filename from the path and emit it
    if (!filePath.isEmpty()) {
        QString fileName = filePath.split('/').last().split('\\').last();
        emit fileNameChanged(fileName);
    }
}