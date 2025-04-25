#include <QApplication>
#include <QQmlApplicationEngine>
#include <QQuickView>
#include <QUrl>
#include <QQmlContext>
#include <iostream>
#include "include/controller.h"

int main(int argc, char *argv[])
{
    std::cout << "Starting application" << std::endl;
    QApplication app(argc, argv);

    // Create the ButtonController instance
    ButtonController buttonController;

    // Create the QQmlApplicationEngine
    QQmlApplicationEngine engine;

    // Register the ButtonController with QML
    engine.rootContext()->setContextProperty("buttonController", &buttonController);

    // Load the QML file from resources
    engine.load(QUrl(QStringLiteral("qrc:/inputScreen.qml")));

    // Check for errors
    if (engine.rootObjects().isEmpty()) {
        std::cerr << "Failed to load QML file!" << std::endl;
        return -1;
    }

    std::cout << "QML loaded successfully" << std::endl;
    return app.exec();
}
