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
            border.color: dndActive ? "#10b981" : "#e5e7eb"
            radius: 10

            property bool dndActive: false

            DropArea {
                anchors.fill: parent
                keys: ["text/uri-list"]

                onEntered: {
                    uploadArea.dndActive = true;
                    console.log("Entered DropArea");
                }
                onExited: {
                    uploadArea.dndActive = false;
                    console.log("Exited DropArea");
                }
                onDropped: {
                    uploadArea.dndActive = false;
                    console.log("File dropped!");

                    if (drop.hasUrls) {
                        let fileUrl = drop.urls[0].toLocalFile();
                        console.log("Dropped file path:", fileUrl);

                        if (fileUrl.endsWith(".txt")) {
                            fileInputController.loadFile(fileUrl);
                        } else {
                            console.warn("Only .txt files are supported.");
                            showErrorMessage("Only .txt files are supported. Please upload a valid course list.");
                        }
                    } else {
                        console.log("Drop has no URLs");
                    }
                }

                MouseArea {
                    anchors.fill: parent
                    acceptedButtons: Qt.NoButton
                }
            }

            // Prompt Text
            Label {
                anchors.centerIn: parent
                text: "Drag and drop your file here,\nor click 'Browse Files'"
                font.pixelSize: 16
                horizontalAlignment: Text.AlignHCenter
                wrapMode: Text.WordWrap
                color: "#6b7280"
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