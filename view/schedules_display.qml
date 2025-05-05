import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15
import Qt.labs.qmlmodels 1.0

Page {
    id: schedulesDisplayPage

    // Header (כמו שכבר בנית)
    Rectangle {
        id: header
        width: parent.width
        height: 80
        color: "#ffffff"
        border.color: "#e5e7eb"

        Row {
            anchors {
                left: parent.left
                verticalCenter: parent.verticalCenter
                leftMargin: 16
            }
            spacing: 16

            Button {
                id: backButton
                width: 40
                height: 40
                background: Rectangle {
                    color: "#f3f4f6"
                    radius: 4
                }
                contentItem: Text {
                    text: "←"
                    font.pixelSize: 18
                    color: "#1f2937"
                    horizontalAlignment: Text.AlignHCenter
                    verticalAlignment: Text.AlignVCenter
                }
                onClicked: {
                    schedulesDisplayController.goBack()
                }
            }

            Label {
                id: titleLabel
                text: "Generated schedules"
                font.pixelSize: 20
                color: "#1f2937"
                anchors.verticalCenter: parent.verticalCenter
            }
        }
    }

    // Layout for content
    ColumnLayout {
        anchors {
            top: header.bottom
            left: parent.left
            right: parent.right
            bottom: parent.bottom
            margins: 24
        }
        spacing: 16

        // Placeholder for schedule index
        Label {
            text: "Schedule 1/1"
            font.pixelSize: 16
            color: "#4b5563"
        }

        // Placeholder content area
        Rectangle {
            Layout.fillWidth: true
            Layout.fillHeight: true
            color: "#f9fafb"
            radius: 8
            border.color: "#d1d5db"

            Text {
                anchors.centerIn: parent
                text: "Schedule content will appear here"
                color: "#6b7280"
                font.pixelSize: 16
            }
        }

        // Navigation buttons row
        RowLayout {
            Layout.alignment: Qt.AlignHCenter
            spacing: 16

            Button {
                text: "< Previous"
                onClicked: schedulesDisplayController.previousSchedule()
            }

            Button {
                text: "Next >"
                onClicked: schedulesDisplayController.nextSchedule()
            }
        }
    }
}