#include "include/main_controller.h"
#include <QQmlContext>

MainController::MainController(QQmlApplicationEngine* engine, QObject *parent)
        : QObject(parent)
        , m_engine(engine)
        , m_fileInputController(new FileInputController(this))
        , m_courseSelectionController(new CourseSelectionController(this))
        , m_schedulesDisplayController(new SchedulesDisplayController(this))
{
    // Connect sub-controller signals to main controller signals
    connectControllerSignals(m_fileInputController);
    connectControllerSignals(m_courseSelectionController);
    connectControllerSignals(m_schedulesDisplayController);

    // Register controllers with QML
    engine->rootContext()->setContextProperty("fileInputController", m_fileInputController);
    engine->rootContext()->setContextProperty("courseSelectionController", m_courseSelectionController);
    engine->rootContext()->setContextProperty("schedulesDisplayController", m_schedulesDisplayController);
}

void MainController::connectControllerSignals(BaseController* controller) {
    // Forward navigation signals from sub-controllers to main controller
    connect(controller, &BaseController::navigateToScreen, this, &MainController::navigateToScreen);
    connect(controller, &BaseController::navigateBack, this, &MainController::navigateBack);
}