#include "../include/Controller.h"
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
    std::cout << "Upload and Continue button clicked - processing file" << std::endl;

    // Navigate to course list screen
    navigateToCourseList();
}

void ButtonController::navigateToCourseList()
{
    // Clear current objects and load course list screen
    m_engine->clearComponentCache();
    m_engine->load(QUrl(QStringLiteral("qrc:/courseListScreen.qml")));
    std::cout << "Navigated to course list screen" << std::endl;
}