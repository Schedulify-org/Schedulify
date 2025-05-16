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

    property string errorDialogText: ""

    Dialog {
        id: errorDialog
        modal: true
        title: ""
        anchors.centerIn: parent
        width: 420
        height: 220
        padding: 0

        // Remove default buttons and handle our own
        standardButtons: Dialog.NoButton
        closePolicy: Dialog.CloseOnEscape

        // Modern clean background
        background: Rectangle {
            color: "#ffffff"
            radius: 8
            border.width: 1
            border.color: "#e5e7eb"
        }

        contentItem: Item {
            width: parent.width
            height: parent.height

            Column {
                id: dialogContent
                anchors.fill: parent
                anchors.margins: 24
                spacing: 16

                // Warning icon (emoji instead of image to avoid resource issues)
                Text {
                    id: errorIcon
                    text: "⚠️"
                    font.pixelSize: 32
                    horizontalAlignment: Text.AlignHCenter
                    width: parent.width
                }

                // Error title
                Label {
                    id: errorTitle
                    text: "File Error"
                    font.pixelSize: 18
                    font.bold: true
                    color: "#1e293b"
                    horizontalAlignment: Text.AlignHCenter
                    width: parent.width
                }

                // Error message
                Label {
                    id: errorMessageView
                    text: errorDialogText
                    wrapMode: Text.WordWrap
                    font.pixelSize: 14
                    color: "#64748b"
                    horizontalAlignment: Text.AlignHCenter
                    width: parent.width
                }
            }

            // Confirm button
            Button {
                id: confirmButton
                anchors {
                    bottom: parent.bottom
                    bottomMargin: 24
                    horizontalCenter: parent.horizontalCenter
                }

                width: 100
                height: 36

                background: Rectangle {
                    radius: 4
                    color: confirmButton.pressed ? "#1e293b" : "#1f2937"
                    border.width: 0
                }

                contentItem: Text {
                    text: "OK"
                    color: "#ffffff"
                    font.pixelSize: 14
                    font.bold: true
                    horizontalAlignment: Text.AlignHCenter
                    verticalAlignment: Text.AlignVCenter
                }

                onClicked: {
                    errorDialog.close()
                }
            }
        }
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