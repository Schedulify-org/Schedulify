#include "main_controller.h"
#include <QQmlContext>

MainController::MainController(QQmlApplicationEngine* engine, QObject *parent)
        : QObject(parent)
        , m_engine(engine)
        , m_courseModel(new CourseModel(this))
{
    // Initialize the course model with sample data
    m_courseModel->populateSampleData();
}

void MainController::handleUploadAndContinue(){
    // Emit signal to navigate to course list screen
    goToScreen(QUrl(QStringLiteral("qrc:/course_selection.qml")));
}

void MainController::goBack()
{
    // Emit signal to go back to previous screen
    emit navigateBack();
}

void MainController::goToScreen(const QUrl &screenUrl)
{
    // Emit signal to navigate to course list screen
    emit navigateToScreen(screenUrl);
}