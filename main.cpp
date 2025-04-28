#include <QApplication>
#include <QQmlApplicationEngine>
#include <QQuickView>
#include <QUrl>
#include <QQmlContext>
#include <iostream>
#include "include/main_controller.h"

int main(int argc, char *argv[])
{
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
        std::cerr << "Failed to load QML file!" << std::endl;
        return -1;
    }

    std::cout << "QML loaded successfully" << std::endl;
    return app.exec();
}