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

// Connect signals from sub-controllers to main controller signals
void MainController::connectControllerSignals(BaseController* controller) {
    // Connect the navigateToScreen signal
    connect(controller, &BaseController::navigateToScreen,
            this, &MainController::navigateToScreen);

    // Connect the navigateBack signal
    connect(controller, &BaseController::navigateBack,
            this, &MainController::navigateBack);
}