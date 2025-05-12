#include "include/main_controller.h"
#include <QQmlContext>

MainController::MainController(QQmlApplicationEngine* engine, QObject *parent)
        : QObject(parent)
        , m_engine(engine)
        , m_fileInputController(new FileInputController(this))
        , m_courseSelectionController(new CourseSelectionController(this))
        , m_schedulesDisplayController(new SchedulesDisplayController(this))
        , m_logDisplayController(new LogDisplayController(this))
{
    // Connect sub-controller signals to main controller signals
    connectControllerSignals(m_fileInputController);
    connectControllerSignals(m_courseSelectionController);
    connectControllerSignals(m_schedulesDisplayController);
    connectControllerSignals(m_logDisplayController);

    // Register controllers with QML
    engine->rootContext()->setContextProperty("fileInputController", m_fileInputController);
    engine->rootContext()->setContextProperty("courseSelectionController", m_courseSelectionController);
    engine->rootContext()->setContextProperty("schedulesDisplayController", m_schedulesDisplayController);
    engine->rootContext()->setContextProperty("logDisplayController", m_logDisplayController);
}

void MainController::connectControllerSignals(const ControllerManager* controller) const {
    // Forward navigation signals from sub-controllers to main controller
    connect(controller, &ControllerManager::navigateToScreen, this, &MainController::navigateToScreen);
    connect(controller, &ControllerManager::navigateBack, this, &MainController::navigateBack);
}