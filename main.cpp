#include <QApplication>
#include <QQmlApplicationEngine>
#include <QQuickView>
#include <QUrl>
#include <QQmlContext>
#include "include/main_controller.h"
#include "main/main_model_menu.h"
#include "logs/logger.h"

int main(int argc, char *argv[])
{
    Logger::get().initialize();

    app_main("COURSES");
    app_main("SCHEDULES");

    QApplication app(argc, argv);

    // Create the QQmlApplicationEngine
    QQmlApplicationEngine engine;

    // Create the ButtonController instance and pass the engine
    MainController controller(&engine);

    // Register the ButtonController with QML
    engine.rootContext()->setContextProperty("controller", &controller);

    // Load the QML file from resources
    engine.load(QUrl(QStringLiteral("qrc:/main.qml")));

    // Check for errors
    if (engine.rootObjects().isEmpty()) {
        return -1;
    }

    return app.exec();
}