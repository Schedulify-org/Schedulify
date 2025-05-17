import QtQuick 2.15
import QtQuick.Layouts 1.15
import QtQuick.Controls 2.15
import QtQuick.Controls.Basic
import "."

Page {
    id: inputScreen

    Connections {
        target: fileInputController

        function onInvalidFileFormat() {
            showErrorMessage("Invalid file format. Please upload a valid course list.");
        }

        function onErrorMessage(message) {
            showErrorMessage(message);
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

            Button {
                id: logButtonA
                anchors {
                    right: parent.right
                    verticalCenter: parent.verticalCenter
                    rightMargin: 15
                }
                width: 40
                height: 40

                background: Rectangle {
                    color: logMouseArea.containsMouse ? "#f3f4f6" : "#ffffff"
                    radius: 20

                    Text {
                        text: "📋"
                        anchors.centerIn: parent
                        font.pixelSize: 20
                    }
                }

                MouseArea {
                    id: logMouseArea
                    anchors.fill: parent
                    hoverEnabled: true
                    cursorShape: Qt.PointingHandCursor
                    onClicked: {
                        if (!logDisplayController.isLogWindowOpen) {
                            var component = Qt.createComponent("qrc:/log_display.qml");
                            if (component.status === Component.Ready) {
                                logDisplayController.setLogWindowOpen(true);
                                var logWindow = component.createObject(inputScreen, {
                                    "onClosing": function(close) {
                                        logDisplayController.setLogWindowOpen(false);
                                    }
                                });
                                logWindow.show();
                            } else {
                                console.error("Error creating log window:", component.errorString());
                            }
                        }
                    }
                }

                ToolTip {
                    visible: logMouseArea.containsMouse
                    text: "Open Application Logs"
                    font.pixelSize: 12
                    delay: 500
                }
            }

        }

        // Upload Container
        Rectangle {
            id: uploadArea
            anchors.centerIn: parent
            width: root ? root.width - 100 : 500
            height: root ? root.height - 400 : 300
            color: "#ffffff"
            border.width: 2
            border.color: "#d1d5db"
            radius: 10

            // Add DropArea to handle file drops
            DropArea {
                id: dropArea
                anchors.fill: parent

                onEntered: function(drag) {
                    uploadArea.border.color = "#4f46e5"
                    uploadArea.border.width = 3
                }

                onExited: function() {
                    uploadArea.border.color = "#d1d5db"
                    uploadArea.border.width = 2
                }

                onDropped: function(drop) {
                    uploadArea.border.color = "#d1d5db"
                    uploadArea.border.width = 2

                    if (drop.hasUrls) {
                        let fileUrl = drop.urls[0];
                        let filePath;

                        // Handle platform differences in file URLs
                        if (fileUrl.toString().startsWith("file:///")) {
                            if (fileUrl.toString().match(/^file:\/\/\/[A-Za-z]:/)) {
                                // Windows path
                                filePath = fileUrl.toString().replace("file:///", "");
                            } else {
                                // Linux/Unix path
                                filePath = fileUrl.toString().replace("file://", "");
                            }
                        } else {
                            // Fallback for other formats
                            filePath = fileUrl.toString();
                        }

                        fileInputController.handleFileSelected(filePath);
                    } else {
                        showErrorMessage("No valid file was dropped.");
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
                    horizontalCenter: parent.horizontalCenter
                    verticalCenter: parent.verticalCenter
                    bottomMargin: 16
                }

                Text {
                    anchors.centerIn: parent
                    text: "📁"
                    font.pixelSize: 48
                    anchors.verticalCenterOffset: -20
                    anchors.horizontalCenterOffset: 0
                    color: "#9ca3af"
                }

                // Drag and Drop Prompt
                Label {
                    id: dropPrompt
                    anchors.centerIn: parent
                    text: "Drag and drop your file here, or click here"
                    anchors.verticalCenterOffset: 40
                    anchors.horizontalCenterOffset: 0
                    color: "#6b7280"
                }

                // File name display (initially hidden)
                Label {
                    id: fileNameText
                    anchors.centerIn: parent
                    color: "#4f46e5"
                    anchors.verticalCenterOffset: 40
                    anchors.horizontalCenterOffset: 0
                    font.bold: true
                    visible: false
                }

                // Browse Button
                Button {
                    id: browseButton
                    anchors {
                        top: fileNameText ? fileNameText.bottom : dropPrompt
                        horizontalCenter: fileNameText ? fileNameText.horizontalCenter : dropPrompt
                        topMargin: 20
                    }
                    background: Rectangle {
                        color: browseMouseArea.containsMouse ? "#35455c" : "#1f2937"
                        radius: 4
                        implicitWidth: 120
                        implicitHeight: 40
                    }
                    font.bold: true
                    contentItem: Text {
                        text: "Browse Files"
                        color: "white"
                        horizontalAlignment: Text.AlignHCenter
                        verticalAlignment: Text.AlignVCenter
                    }
                    MouseArea {
                        id: browseMouseArea
                        anchors.fill: parent
                        hoverEnabled: true
                        onClicked: fileInputController.handleUploadAndContinue()
                        cursorShape: Qt.PointingHandCursor
                    }
                }

                // Supported Formats Text
                Label {
                    id: supportedFormats
                    anchors {
                        top: browseButton.bottom
                        horizontalCenter: browseButton.horizontalCenter
                        topMargin: 16
                    }
                    text: "Supported formats: TXT"
                    font.pixelSize: 12
                    color: "#9ca3af"
                }
            }
        }

        // Upload Button - Initially hidden
        Button {
            id: continueButton
            width: 194
            height: 40
            visible: false  // Initially hidden until a file is selected
            anchors {
                top: uploadArea.bottom
                horizontalCenter: uploadArea.horizontalCenter
                topMargin: 8
            }
            background: Rectangle {
                color: generateCoursesMouseArea.containsMouse ? "#35455c" : "#1f2937"
                radius: 4
                implicitWidth: 120
                implicitHeight: 40
            }
            font.bold: true
            contentItem: Text {
                text: "Upload Course's List →"
                color: "white"
                horizontalAlignment: Text.AlignHCenter
                verticalAlignment: Text.AlignVCenter
            }
            MouseArea {
                id: generateCoursesMouseArea
                anchors.fill: parent
                hoverEnabled: true
                onClicked: fileInputController.loadFile()
                cursorShape: Qt.PointingHandCursor
            }
        }

        // Upload Title Label
        Label {
            id: uploadTitle
            y: 239
            anchors {
                bottom: uploadArea.top
                left: uploadArea.left
                bottomMargin: 29
            }
            text: "Upload Your Course List"
            anchors.leftMargin: 0
            font.pixelSize: 24
            color: "#1f2937"
        }

        // Upload Description
        Label {
            id: uploadDescription
            x: 50
            y: 177
            anchors {
                top: uploadTitle.bottom
                left: uploadTitle.left
                bottomMargin: 16
            }
            text: "Upload your course file to start building your schedule"
            color: "#6b7280"
        }

        // footer
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
                text: "© 2025 Schedulify. All rights reserved."
                color: "#6b7280"
                font.pixelSize: 12
            }
        }
    }
}
