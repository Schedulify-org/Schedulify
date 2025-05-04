import QtQuick 2.15
import QtQuick.Controls 2.15

Window {
    id: mainWindow
    visible: true
    width: 1024
    height: 768
    minimumWidth: 1024
    minimumHeight: 768
    title: "Schedule Builder"

    // StackView to handle screen transitions
    StackView {
        id: stackView
        anchors.fill: parent
        initialItem: "qrc:/file_input.qml"
    }

    // Connect to the controller's signals for navigation
    Connections {
        target: controller

        function onNavigateToScreen(screenUrl) {
            stackView.push(screenUrl);
        }

        function onNavigateBack() {
            if (stackView.depth > 1) {
                stackView.pop();
            }
        }
    }
}