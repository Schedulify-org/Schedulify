import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15

Page {
    id: inputScreen

    Connections {
        target: fileInputController

        function onInvalidFileFormat() {
            showErrorMessage("Invalid file format. Please upload a valid course list.");
        }

        // Connection for fileSelected signal
        function onFileSelected(hasFile) {
            continueButton.visible = hasFile;
        }

        // Connection for fileNameChanged signal
        function onFileNameChanged(fileName) {
            fileNameText.text = fileName;
            fileNameText.visible = true;
            dropPrompt.visible = false;
        }
    }

    width: 1024
    height: 768

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
                    id: errorMessage
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
                    text: "אישור"  // Hebrew for "OK/Confirm"
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

    function showErrorMessage(msg) {
        errorDialogText = msg;
        errorDialog.open();
    }

    Rectangle {
        id: root
        anchors.fill: parent
        color: "#f9fafb"

        // header
        Rectangle {
            id: header
            width: parent.width
            height: 80
            color: "#ffffff"
            border.color: "#e5e7eb"

            // Title Label
            Label {
                id: titleLabel
                x: 16
                y: 28
                text: "Schedule Builder"
                font.pixelSize: 20
                color: "#1f2937"
            }
        }

        // Upload Container
        Rectangle {
            id: uploadArea
            anchors.centerIn: parent
            width: 500
            height: 300
            color: "#ffffff"
            border.width: 2
            border.color: "#d1d5db"
            radius: 10

            // Add DropArea to handle file drops
            DropArea {
                id: dropArea
                anchors.fill: parent

                // Visual feedback when dragging over the area
                onEntered: {
                    uploadArea.border.color = "#4f46e5" // Change border color when dragging over
                    uploadArea.border.width = 3
                }

                onExited: {
                    uploadArea.border.color = "#d1d5db" // Restore border color
                    uploadArea.border.width = 2
                }

                onDropped: {
                    uploadArea.border.color = "#d1d5db"
                    uploadArea.border.width = 2

                    // Check if the drop has URLs
                    if (drop.hasUrls) {
                        // Get the first file URL
                        var fileUrl = drop.urls[0]
                        // Remove the "file:///" prefix to get the file path
                        var filePath = fileUrl.toString().replace(/^(file:\/{3})/, "")

                        // Call the controller with the file path - UI will be updated via signals
                        fileInputController.handleFileSelected(filePath)
                    }
                }
            }

            // Upload icon
            Rectangle {
                id: uploadIcon
                width: 64
                height: 64
                color: "transparent"
                anchors {
                    bottom: dropPrompt.top
                    horizontalCenter: parent.horizontalCenter
                    bottomMargin: 16
                }

                Text {
                    anchors.centerIn: parent
                    text: "📁"
                    font.pixelSize: 48
                    color: "#9ca3af"
                }
            }

            // Drag and Drop Prompt
            Label {
                id: dropPrompt
                anchors.centerIn: parent
                text: "Drag and drop your file here, or"
                color: "#6b7280"
            }

            // File name display (initially hidden)
            Label {
                id: fileNameText
                anchors.centerIn: parent
                color: "#4f46e5"
                font.bold: true
                visible: false
            }
        }

        // Upload Title Label
        Label {
            id: uploadTitle
            anchors {
                bottom: uploadArea.top
                left: uploadArea.left
                bottomMargin: 16
            }
            text: "Upload Your Course List"
            font.pixelSize: 24
            color: "#1f2937"
        }

        // Upload Description
        Label {
            id: uploadDescription
            anchors {
                top: uploadTitle.bottom
                left: uploadTitle.left
                bottomMargin: 16
            }
            text: "Upload your course file to start building your schedule"
            color: "#6b7280"
        }

        // Supported Formats Text
        Label {
            id: supportedFormats
            anchors {
                top: uploadArea.bottom
                horizontalCenter: uploadArea.horizontalCenter
                topMargin: 16
            }
            text: "Supported formats: TXT"
            font.pixelSize: 12
            color: "#9ca3af"
        }

        // Browse Button
        Button {
            id: browseButton
            anchors {
                top: supportedFormats.bottom
                horizontalCenter: supportedFormats.horizontalCenter
                topMargin: 8
            }
            background: Rectangle {
                color: "#1f2937"
                radius: 4
                implicitWidth: 120
                implicitHeight: 40
            }
            font.bold: true
            contentItem: Text {
                text: qsTr("Browse Files")
                color: "white"
                horizontalAlignment: Text.AlignHCenter
                verticalAlignment: Text.AlignVCenter
            }
            onClicked: {
                fileInputController.handleUploadAndContinue()
            }
        }

        // Upload Button - Initially hidden
        Button {
            id: continueButton
            width: 194
            height: 40
            visible: false  // Initially hidden until a file is selected
            anchors {
                top: browseButton.bottom
                horizontalCenter: supportedFormats.horizontalCenter
                topMargin: 8
            }
            background: Rectangle {
                color: "#1f2937"
                radius: 4
                implicitWidth: 120
                implicitHeight: 40
            }
            font.bold: true
            contentItem: Text {
                text: qsTr("Upload Course's List →")
                color: "white"
                horizontalAlignment: Text.AlignHCenter
                verticalAlignment: Text.AlignVCenter
            }
            onClicked: {
                fileInputController.loadFile()
            }
        }

        // bottom row
        Rectangle {
            id: footer
            anchors {
                bottom: parent.bottom
                left: parent.left
                right: parent.right
            }
            height: 60
            color: "#ffffff"
            border.color: "#e5e7eb"

            // Footer Text
            Label {
                anchors.centerIn: parent
                text: "© 2025 Schedule Builder. All rights reserved."
                color: "#6b7280"
                font.pixelSize: 12
            }
        }
    }
}
