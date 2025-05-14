#include "controller_manager.h"
#include "main_controller.h" // Include the main controller header

// Initialize static member
MainController* ControllerManager::s_mainController = nullptr;

void ControllerManager::setMainController(MainController* controller) {
    s_mainController = controller;
}

QObject* ControllerManager::findController(const QString& controllerName) {
    if (!s_mainController) {
        qWarning() << "MainController not set, cannot find controller" << controllerName;
        return nullptr;
    }

    // Use controller name to return the appropriate controller
    if (controllerName == "fileInputController") {
        return s_mainController->fileInputController();
    }
    else if (controllerName == "courseSelectionController") {
        return s_mainController->courseSelectionController();
    }
    else if (controllerName == "schedulesDisplayController") {
        return s_mainController->schedulesDisplayController();
    }

    qWarning() << "Unknown controller name:" << controllerName;
    return nullptr;
}

QQmlApplicationEngine *ControllerManager::getEngine() {
    return s_mainController->getEngine();
}
