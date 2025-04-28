import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15

Item {
    id: courseListScreen
    width: 1024
    height: 768

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
                        controller.goBack();
                    }
                }

                // Screen Title
                Label {
                    id: titleLabel
                    text: "Available Courses"
                    font.pixelSize: 20
                    color: "#1f2937"
                    anchors.verticalCenter: parent.verticalCenter
                }
            }
        }

        // Main content area
        Rectangle {
            id: contentArea
            anchors {
                top: header.bottom
                left: parent.left
                right: parent.right
                bottom: footer.top
            }
            color: "#f9fafb"

            Label {
                id: courseListTitle
                anchors {
                    top: parent.top
                    left: parent.left
                    margins: 16
                }
                text: "Course List"
                font.pixelSize: 24
                color: "#1f2937"
            }

            Label {
                id: courseListDescription
                anchors {
                    top: courseListTitle.bottom
                    left: parent.left
                    right: parent.right
                    margins: 16
                }
                text: "Below are the courses available for your schedule"
                color: "#6b7280"
            }

            // ListView for courses
            ListView {
                id: courseListView
                anchors {
                    top: courseListDescription.bottom
                    left: parent.left
                    right: parent.right
                    bottom: parent.bottom
                    margins: 16
                }
                clip: true
                model: controller.courseModel
                spacing: 8

                // Delegate for course items
                delegate: Rectangle {
                    width: ListView.view.width
                    height: 80
                    color: "#ffffff"
                    radius: 8
                    border.color: "#e5e7eb"

                    RowLayout {
                        anchors {
                            fill: parent
                            margins: 12
                        }
                        spacing: 16

                        // Course ID
                        Rectangle {
                            Layout.preferredWidth: 80
                            Layout.preferredHeight: 80 - 24
                            color: "#f3f4f6"
                            radius: 4

                            Label {
                                anchors.centerIn: parent
                                text: courseId
                                font.bold: true
                                color: "#4b5563"
                            }
                        }

                        // Course details
                        Column {
                            Layout.fillWidth: true
                            spacing: 4

                            Label {
                                text: courseName
                                font.pixelSize: 16
                                font.bold: true
                                color: "#1f2937"
                            }

                            Label {
                                text: "Instructor: " + teacherName
                                font.pixelSize: 14
                                color: "#6b7280"
                            }
                        }
                    }
                }

                // Scrollbar
                ScrollBar.vertical: ScrollBar {
                    active: true
                }
            }
        }

        // Footer
        Rectangle {
            id: footer
            width: parent.width
            height: 60
            anchors.bottom: parent.bottom
            color: "#ffffff"
            border.color: "#e5e7eb"

            Label {
                anchors.centerIn: parent
                text: "© 2025 Schedule Builder. All rights reserved."
                color: "#6b7280"
                font.pixelSize: 12
            }
        }
    }
}