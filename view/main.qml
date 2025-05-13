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

    Loader {
        id: overlayLoader
        anchors.fill: parent
        active: false
        source: "qrc:/loading_overlay.qml"
        z: 9999 // Ensure it appears on top of everything

        // Connect the abort signal
        Connections {
            target: overlayLoader.item
            function onAbortRequested() {
                courseSelectionController.abortGeneration()
            }
        }
    }

    // Add this function to show/hide the overlay
    function showLoadingOverlay(show) {
        overlayLoader.active = show
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