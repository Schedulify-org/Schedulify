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
            border.color: "#e5e7eb"
            radius: 10
            state: ""

            // Add state transitions for smooth animation
            transitions: [
                Transition {
                    to: "*"
                    ColorAnimation {
                        target: uploadArea
                        property: "border.color"
                        duration: 200
                    }
                }
            ]

            DropArea {
                id: dropArea
                anchors.fill: parent
                keys: ["text/uri-list"]

                property bool isValidFile: false
                property string fileName: ""

                // Visual state properties
                states: [
                    State {
                        name: "dropped"
                        PropertyChanges { target: uploadArea; border.color: dropArea.isValidFile ? "#10b981" : "#ef4444" }
                        PropertyChanges { target: dropIndicator; text: dropArea.isValidFile ?
                            "File ready: " + dropArea.fileName :
                            "Invalid file format.\nOnly .txt files are supported."
                        }
                        PropertyChanges { target: dropIndicator; color: dropArea.isValidFile ? "#10b981" : "#ef4444" }
                    },
                    State {
                        name: "dragging"
                        PropertyChanges { target: uploadArea; border.color: "#3b82f6" }
                        PropertyChanges { target: dropIndicator; text: "Release to upload file" }
                        PropertyChanges { target: dropIndicator; color: "#3b82f6" }
                    }
                ]

                onEntered: {
                    uploadArea.state = "dragging"
                    console.log("File dragged in")
                }

                onExited: {
                    uploadArea.state = ""
                    console.log("File drag exited")
                }

                onDropped: {
                    console.log("File dropped!")

                    if (drop.hasUrls) {
                        let fileUrl = drop.urls[0]

                        // Convert URL to local file path
                        let localFilePath = fileUrl.toLocalFile()
                        console.log("Dropped file path:", localFilePath)

                        // Extract just the filename for display
                        fileName = localFilePath.substring(localFilePath.lastIndexOf('/') + 1)
                        if (fileName.indexOf('\\') !== -1) {
                            fileName = fileName.substring(fileName.lastIndexOf('\\') + 1)
                        }

                        // Check if it's a .txt file
                        isValidFile = fileName.toLowerCase().endsWith(".txt")

                        if (isValidFile) {
                            uploadArea.state = "dropped"
                            // Call the C++ function to process the file
                            fileInputController.loadFile(localFilePath)
                        } else {
                            uploadArea.state = "dropped"
                            showErrorMessage("Invalid file format. Please upload a .txt file.")
                            console.warn("Invalid file format")
                        }
                    } else {
                        console.log("Drop has no URLs")
                        uploadArea.state = ""
                    }
                }
            }

            // We're removing the MouseArea since we want drag-only functionality for this area

            // Upload content column
            Column {
                id: dropContent
                anchors.centerIn: parent
                spacing: 12
                width: parent.width - 40

                // Upload icon (text fallback if image is missing)
                Text {
                    id: uploadIcon
                    text: uploadArea.state === "dropped" && dropArea.isValidFile ? "✓" : "⬆️"
                    font.pixelSize: 32
                    color: uploadArea.state === "dropped" && dropArea.isValidFile ? "#10b981" : "#6b7280"
                    horizontalAlignment: Text.AlignHCenter
                    width: parent.width
                }

                // Drop text indicator
                Label {
                    id: dropIndicator
                    text: "Drag and drop your file here"
                    font.pixelSize: 16
                    horizontalAlignment: Text.AlignHCenter
                    wrapMode: Text.WordWrap
                    color: "#6b7280"
                    width: parent.width
                }
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