import QtQuick 2.15
import QtQuick.Controls 2.15

Window {
    id: mainWindow
    visible: true
    width: 1024
    height: 768
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
            // Check which screen is being navigated to
            if (screenUrl.toString() === "qrc:/course_selection.qml") {
                // For course selection screen, add a callback
                stackView.push(screenUrl, {
                    "onActivated": function() {
                        courseSelectionController.initialize()
                    }
                });
            } else {
                // For other screens, just push without callback
                stackView.push(screenUrl);
            }
        }

        function onNavigateBack() {
            if (stackView.depth > 1) {
                stackView.pop();
            }
        }
    }
}