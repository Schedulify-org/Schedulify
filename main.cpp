#include <QApplication>
#include <QQmlApplicationEngine>
#include <QQuickView>
#include <QUrl>
#include <QQmlContext>
#include "main_controller.h"
#include "logger.h"

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    Logger::get().logInitiate();

    // Create the QQmlApplicationEngine
    QQmlApplicationEngine engine;

    // Create the ButtonController instance and pass the engine
    MainController controller(&engine);

    // Set the static reference to the main controller for all ControllerManager instances
    ControllerManager::setMainController(&controller);

    // Register the ButtonController with QML
    engine.rootContext()->setContextProperty("controller", &controller);

    // Load the QML file from resources
    engine.load(QUrl(QStringLiteral("qrc:/main.qml")));

    // Check for errors
    if (engine.rootObjects().isEmpty()) {
        Logger::get().logError("Error loading main qml (view) file");
        return -1;
    }

    return app.exec();
}