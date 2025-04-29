#include "file_input.h"

FileInputController::FileInputController(QObject *parent)
        : BaseController(parent){}

void FileInputController::handleUploadAndContinue() {
    // Logic for handling file upload
    // ...

    // Navigate to course selection screen
    goToScreen(QUrl(QStringLiteral("qrc:/course_selection.qml")));
}
