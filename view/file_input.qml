// file_input.qml
import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15
import QtQuick.Dialogs 1.3

Page {
    id: fileInputPage

    header: ToolBar {
        RowLayout {
            anchors.fill: parent

            Label {
                text: "Schedule Builder - File Input"
                font.pixelSize: 20
                elide: Label.ElideRight
                horizontalAlignment: Qt.AlignHCenter
                verticalAlignment: Qt.AlignVCenter
                Layout.fillWidth: true
            }
        }
    }

    ColumnLayout {
        anchors.centerIn: parent
        spacing: 20
        width: parent.width * 0.7

        Label {
            text: "Upload your course data file"
            font.pixelSize: 18
            Layout.alignment: Qt.AlignHCenter
        }

        Rectangle {
            Layout.fillWidth: true
            height: 150
            color: "lightgray"
            border.color: "gray"
            border.width: 2
            radius: 5

            ColumnLayout {
                anchors.centerIn: parent
                spacing: 10

                Label {
                    text: "Drag and drop your file here"
                    font.pixelSize: 16
                    Layout.alignment: Qt.AlignHCenter
                }

                Button {
                    text: "Browse Files"
                    Layout.alignment: Qt.AlignHCenter
                    onClicked: fileDialog.open()
                }
            }
        }

        Button {
            text: "Upload and Continue"
            Layout.alignment: Qt.AlignHCenter
            Layout.preferredWidth: 200
            Layout.preferredHeight: 50
            font.pixelSize: 16
            onClicked: fileInputController.handleUploadAndContinue()
        }
    }

    FileDialog {
        id: fileDialog
        title: "Please choose a file"
        folder: shortcuts.home
        nameFilters: ["CSV files (*.csv)", "All files (*)"]
        onAccepted: {
            console.log("File selected: " + fileDialog.fileUrl)
        }
    }
}
