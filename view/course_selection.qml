import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15

Page {
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
                        courseSelectionController.goBack()
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

            // Selected Courses Row
            Rectangle {
                id: selectedCoursesContainer
                anchors {
                    top: parent.top
                    left: parent.left
                    right: parent.right
                    margins: 16
                }
                height: 90
                color: "#ffffff"
                radius: 8
                border.color: "#e5e7eb"

                Column {
                    anchors {
                        fill: parent
                        margins: 12
                    }
                    spacing: 8

                    Label {
                        text: "Selected Courses"
                        font.pixelSize: 16
                        font.bold: true
                        color: "#1f2937"
                    }

                    Flow {
                        id: selectedCoursesFlow
                        width: parent.width
                        spacing: 8
                        height: 40

                        // This will be populated dynamically with selected courses
                        Repeater {
                            id: selectedCoursesRepeater
                            model: courseSelectionController.selectedCoursesModel

                            Rectangle {
                                width: courseIdText.width + 36
                                height: 36
                                radius: 4
                                color: "#4f46e5"

                                Label {
                                    id: courseIdText
                                    anchors.centerIn: parent
                                    text: courseId
                                    font.bold: true
                                    color: "#ffffff"
                                }

                                MouseArea {
                                    anchors.fill: parent
                                    onClicked: {
                                        courseSelectionController.deselectCourse(index)
                                    }
                                }
                            }
                        }
                    }
                }
            }

            Label {
                id: courseListTitle
                anchors {
                    top: selectedCoursesContainer.bottom
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
                text: "Click on a course to select it for your schedule"
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
                model: courseSelectionController.courseModel
                spacing: 8

                // Delegate for course items
                delegate: Rectangle {
                    id: courseDelegate
                    width: ListView.view.width
                    height: 80
                    color: courseSelectionController.isCourseSelected(index) ? "#f0f9ff" : "#ffffff" // Highlight if selected
                    radius: 8
                    border.color: courseSelectionController.isCourseSelected(index) ? "#3b82f6" : "#e5e7eb"

                    // Using a connection to force update when selection changes
                    Connections {
                        target: courseSelectionController
                        function onSelectionChanged() {
                            courseDelegate.color = courseSelectionController.isCourseSelected(index) ? "#f0f9ff" : "#ffffff"
                            courseDelegate.border.color = courseSelectionController.isCourseSelected(index) ? "#3b82f6" : "#e5e7eb"
                        }
                    }

                    RowLayout {
                        anchors {
                            fill: parent
                            margins: 12
                        }
                        spacing: 16

                        // Course ID
                        Rectangle {
                            id: courseIdBox
                            Layout.preferredWidth: 80
                            Layout.preferredHeight: 80 - 24
                            color: courseSelectionController.isCourseSelected(index) ? "#dbeafe" : "#f3f4f6"
                            radius: 4

                            Label {
                                anchors.centerIn: parent
                                text: courseId
                                font.bold: true
                                color: courseSelectionController.isCourseSelected(index) ? "#2563eb" : "#4b5563"
                            }

                            // Update color when selection changes
                            Connections {
                                target: courseSelectionController
                                function onSelectionChanged() {
                                    courseIdBox.color = courseSelectionController.isCourseSelected(index) ? "#dbeafe" : "#f3f4f6"
                                    courseIdBox.children[0].color = courseSelectionController.isCourseSelected(index) ? "#2563eb" : "#4b5563"
                                }
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

                    MouseArea {
                        anchors.fill: parent
                        onClicked: {
                            courseSelectionController.toggleCourseSelection(index)
                        }
                    }
                }

                // Scrollbar
                ScrollBar.vertical: ScrollBar {
                    active: true
                }
            }
        }

        // Generate Schedules Button
        Button {
            id: generateButton
            anchors {
                right: parent.right
                bottom: footer.top
                margins: 16
            }
            background: Rectangle {
                color: "#1f2937"
                radius: 4
                implicitWidth: 180
                implicitHeight: 40
            }
            font.bold: true
            contentItem: Text {
                text: qsTr("Generate Schedules")
                color: "white"
                horizontalAlignment: Text.AlignHCenter
                verticalAlignment: Text.AlignVCenter
            }
            onClicked: {
                courseSelectionController.generateSchedules()
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