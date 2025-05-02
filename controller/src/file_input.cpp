#include "file_input.h"
#include <QFileDialog>
#include <QStandardPaths>
#include <QDebug>
#include "../../model/include/main/main_model.h"
#include "../include/course_selection.h" // include CourseSelectionController header

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

    // Proceed to load the selected file
    loadFile(fileName);
}

void FileInputController::loadFile(const QString &filePath) {
    qDebug() << "[C++] loadFile called with path:" << filePath;

    Model model;

    auto* coursesPtr = static_cast<std::vector<Course>*>(
            model.executeOperation(ModelOperation::GENERATE_COURSES, nullptr, filePath.toStdString())
    );

    if (!coursesPtr) {
        qWarning() << "[C++] coursesPtr is nullptr!";
        emit invalidFileFormat();  // 🔔 Notify QML
        return;
    }

    std::vector<Course> &courses = *coursesPtr;
    qDebug() << "[C++] Parsed course count:" << static_cast<int>(courses.size());

    auto* course_controller =
            qobject_cast<CourseSelectionController*>(findController("courseSelectionController"));

    if (!course_controller) {
        qWarning() << "[C++] CourseSelectionController not found!";
        emit invalidFileFormat();  // 🔔 Notify QML
        return;
    }

    if (!courses.empty()) {
        qDebug() << "[C++] Initiating courses and switching screen...";
        course_controller->initiateCoursesData(courses);
        goToScreen(QUrl(QStringLiteral("qrc:/course_selection.qml")));
    } else {
        qWarning() << "[C++] No courses loaded from file!";
        emit invalidFileFormat();  // 🔔 Notify QML
    }
}
