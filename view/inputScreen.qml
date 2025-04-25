import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts
import QtQuick.Dialogs

Window {
    visible: true
    width: 1024
    height: 768
    title: "Schedule Builder"

    Rectangle {
        id: root
        width: 1024
        height: 768
        color: "#f9fafb"

        // ==== Layout Container ====
        Rectangle {
            id: header
            x: 0
            y: 0
            width: 1024
            height: 80
            color: "#ffffff"
            border.color: "#e5e7eb"
        }

        Rectangle {
            id: uploadArea
            x: 269
            y: 240
            width: 500
            height: 300
            color: "#ffffff"
            border.width: 2
            border.color: "#d1d5db"
            radius: 10
        }

        Rectangle {
            id: footer
            x: 0
            y: 708
            width: 1024
            height: 60
            color: "#ffffff"
            border.color: "#e5e7eb"
        }

        // Footer Text
        Label {
            id: footerText
            x: 404
            y: 722
            anchors.centerIn: parent
            text: "© 2025 Schedule Builder. All rights reserved."
            color: "#6b7280"
            font.pixelSize: 12
        }

        // Title Label
        Label {
            id: titleLabel
            x: 16
            y: 28
            text: "Schedule Builder"
            font.family: "FontAwesome"
            font.pixelSize: 20
            color: "#1f2937"
        }

        // Upload Title Label
        Label {
            id: uploadTitle
            x: 269
            y: 159
            text: "Upload Your Course List"
            font.pixelSize: 24
            color: "#1f2937"
        }

        // Upload Description
        Label {
            id: uploadDescription
            x: 275
            y: 207
            text: "Upload your course file to start building your schedule"
            color: "#6b7280"
        }

        // Drag and Drop Prompt
        Label {
            id: dropPrompt
            x: 430
            y: 349
            text: "Drag and drop your file here, or"
            color: "#6b7280"
        }

        // Browse Button
        Button {
            id: browseButton
            x: 475
            y: 498
            background: Rectangle {
                color: "#1f2937"
                radius: 4
            }
            font.bold: true
            contentItem: Text {
                text: qsTr("Browse Files")
                color: "white"
            }
            onClicked: {
                // Call the C++ controller method
                buttonController.handleButtonClicked("Browse Files");
                buttonController.handleBrowseFiles();
            }
        }

        // Supported Formats Text
        Label {
            id: supportedFormats
            x: 434
            y: 476
            text: "Supported formats: CSV, Excel"
            font.pixelSize: 12
            color: "#9ca3af"
        }

        // Upload and Continue Button
        Button {
            id: uploadSubmitButton
            x: 437
            y: 645
            width: 150
            text: "Upload and Continue"
            background: Rectangle {
                color: "#1f2937"
                radius: 4
            }
            contentItem: Text {
                text: qsTr("Upload and Continue")
                color: "white"
            }
            onClicked: {
                // Call the C++ controller method
                buttonController.handleButtonClicked("Upload and Continue");
                buttonController.handleUploadAndContinue();
            }
        }
    }
}