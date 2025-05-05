import QtQuick 2.15
import QtQuick.Layouts 1.15
import Qt.labs.qmlmodels 1.0
import QtQuick.Controls 2.15
import QtQuick.Controls.Basic

Page {
    id: schedulesDisplayPage

    Rectangle {
        id: root
        anchors.fill: parent
        color: "#f9fafb"

        // Header
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

                // Back Button
                Button {
                    id: schedulesBackButton
                    width: 40
                    height: 40
                    anchors {
                        left: parent.left
                        leftMargin: 16
                        verticalCenter: parent.verticalCenter
                    }
                    background: Rectangle {
                        color: schedulesBackMouseArea.containsMouse ? "#a8a8a8" : "#f3f4f6"
                        radius: 4
                    }
                    contentItem: Text {
                        text: "←"
                        font.pixelSize: 18
                        color: "#1f2937"
                        horizontalAlignment: Text.AlignHCenter
                        verticalAlignment: Text.AlignVCenter
                    }
                    MouseArea {
                        id: schedulesBackMouseArea
                        anchors.fill: parent
                        hoverEnabled: true
                        onClicked: schedulesDisplayController.goBack()
                        cursorShape: Qt.PointingHandCursor
                    }
                }

                // Screen Title
                Label {
                    id: titleLabel
                    text: "Generated schedules"
                    font.pixelSize: 20
                    color: "#1f2937"
                    anchors {
                        left: schedulesBackButton.right
                        leftMargin: 16
                        verticalCenter: parent.verticalCenter
                    }
                }
            }
        }

        // Layout for content
        ColumnLayout {
            anchors {
                top: header.bottom
                left: parent.left
                right: parent.right
                bottom: footer.top
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
                    id: prevButton
                    width: 180
                    height: 40
                    background: Rectangle {
                        color: prevMouseArea.containsMouse ? "#35455c" : "#1f2937"
                        radius: 4
                        implicitWidth: 180
                        implicitHeight: 40
                    }
                    font.bold: true
                    contentItem: Text {
                        text: "← Previous"
                        color: "white"
                        horizontalAlignment: Text.AlignHCenter
                        verticalAlignment: Text.AlignVCenter
                    }
                    MouseArea {
                        id: prevMouseArea
                        anchors.fill: parent
                        hoverEnabled: true
                        onClicked: schedulesDisplayController.previousSchedule()
                        cursorShape: Qt.PointingHandCursor
                    }
                }

                Button {
                    id: nextButton
                    width: 180
                    height: 40
                    background: Rectangle {
                        color: forwardMouseArea.containsMouse ? "#35455c" : "#1f2937"
                        radius: 4
                        implicitWidth: 180
                        implicitHeight: 40
                    }
                    font.bold: true
                    contentItem: Text {
                        text: "Next →"
                        color: "white"
                        horizontalAlignment: Text.AlignHCenter
                        verticalAlignment: Text.AlignVCenter
                    }
                    MouseArea {
                        id: forwardMouseArea
                        anchors.fill: parent
                        hoverEnabled: true
                        onClicked: schedulesDisplayController.nextSchedule()
                        cursorShape: Qt.PointingHandCursor
                    }
                }
            }
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
            width: parent.width
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