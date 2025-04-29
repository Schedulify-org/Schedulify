// file_input.qml
import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15
import QtQuick.Dialogs 1.3

Item {
    id: fileInputPage
    width: 1024
    height: 768

    Rectangle {
        id: root
        anchors.fill: parent
        color: "#f9fafb"

        // ==== Layout Container ====
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

            // Drag and Drop Prompt
            Label {
                id: dropPrompt
                anchors.centerIn: parent
                text: "Drag and drop your file here, or"
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
            text: "Supported formats: CSV, Excel"
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
                controller.handleUploadAndContinue();
            }
        }

        // Upload and Continue Button
        Button {
            id: uploadSubmitButton
            anchors {
                top: browseButton.bottom
                horizontalCenter: browseButton.horizontalCenter
                topMargin: 16
            }
            background: Rectangle {
                color: "#1f2937"
                radius: 4
                implicitWidth: 180
                implicitHeight: 40
            }
            contentItem: Text {
                text: qsTr("Upload and Continue")
                color: "white"
                horizontalAlignment: Text.AlignHCenter
                verticalAlignment: Text.AlignVCenter
            }
            onClicked: {
                controller.handleUploadAndContinue();
            }
        }

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

// Page {
//     id: fileInputPage
//
//     header: ToolBar {
//         RowLayout {
//             anchors.fill: parent
//
//             Label {
//                 text: "Schedule Builder - File Input"
//                 font.pixelSize: 20
//                 elide: Label.ElideRight
//                 horizontalAlignment: Qt.AlignHCenter
//                 verticalAlignment: Qt.AlignVCenter
//                 Layout.fillWidth: true
//             }
//         }
//     }
//
//     ColumnLayout {
//         anchors.centerIn: parent
//         spacing: 20
//         width: parent.width * 0.7
//
//         Label {
//             text: "Upload your course data file"
//             font.pixelSize: 18
//             Layout.alignment: Qt.AlignHCenter
//         }
//
//         Rectangle {
//             Layout.fillWidth: true
//             height: 150
//             color: "lightgray"
//             border.color: "gray"
//             border.width: 2
//             radius: 5
//
//             ColumnLayout {
//                 anchors.centerIn: parent
//                 spacing: 10
//
//                 Label {
//                     text: "Drag and drop your file here"
//                     font.pixelSize: 16
//                     Layout.alignment: Qt.AlignHCenter
//                 }
//
//                 Button {
//                     text: "Browse Files"
//                     Layout.alignment: Qt.AlignHCenter
//                     onClicked: fileDialog.open()
//                 }
//             }
//         }
//
//         Button {
//             text: "Upload and Continue"
//             Layout.alignment: Qt.AlignHCenter
//             Layout.preferredWidth: 200
//             Layout.preferredHeight: 50
//             font.pixelSize: 16
//             onClicked: fileInputController.handleUploadAndContinue()
//         }
//     }
//
//     FileDialog {
//         id: fileDialog
//         title: "Please choose a file"
//         folder: shortcuts.home
//         nameFilters: ["CSV files (*.csv)", "All files (*)"]
//         onAccepted: {
//             console.log("File selected: " + fileDialog.fileUrl)
//         }
//     }
// }
