#include "../include/controller.h"
#include <QQmlContext>

ButtonController::ButtonController(QQmlApplicationEngine* engine, QObject *parent)
        : QObject(parent)
        , m_engine(engine)
        , m_courseModel(new CourseModel(this))
{
    std::cout << "ButtonController initialized" << std::endl;

    // Initialize the course model with sample data
    m_courseModel->populateSampleData();
}

ButtonController::~ButtonController()
{
    // Destructor implementation
}

void ButtonController::handleButtonClicked(const QString &buttonName)
{
    std::cout << "Button clicked: " << buttonName.toStdString() << std::endl;
}

void ButtonController::handleBrowseFiles()
{
    std::cout << "Browse Files button clicked - opening file dialog" << std::endl;
    // Here you would implement file dialog functionality
}

void ButtonController::handleUploadAndContinue()
{
    std::cout << "Upload and Continue button clicked - navigating to course list" << std::endl;

    // Navigate to course list screen
    goToCourseListScreen();
}

void ButtonController::goToCourseListScreen()
{
    // Emit signal to navigate to course list screen
    emit navigateToScreen(QUrl(QStringLiteral("qrc:/courseListScreen.qml")));
    std::cout << "Navigated to course list screen" << std::endl;
}

void ButtonController::goBack()
{
    // Emit signal to go back to previous screen
    emit navigateBack();
    std::cout << "Navigated back to previous screen" << std::endl;
}