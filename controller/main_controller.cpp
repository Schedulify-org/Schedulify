#include "main_controller.h"

MainController::MainController(QQmlApplicationEngine* engine, QObject *parent)
        : QObject(parent)
        , m_engine(engine)
        , m_fileInputController(new FileInputController(this))
        , m_courseSelectionController(new CourseSelectionController(this))
        , m_schedulesDisplayController(new SchedulesDisplayController(this))
{
    // Connect signals from sub-controllers to main controller
    connectControllerSignals(m_fileInputController);
    connectControllerSignals(m_courseSelectionController);
    connectControllerSignals(m_schedulesDisplayController);
}

void MainController::connectControllerSignals(BaseController* controller) const {
    // Forward navigation signals from sub-controllers to main controller
    connect(controller, &BaseController::navigateToScreen, this, &MainController::navigateToScreen);
    connect(controller, &BaseController::navigateBack, this, &MainController::navigateBack);
}