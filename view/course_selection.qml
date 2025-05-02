import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15

Page {
    id: courseListScreen

    // Property to store error message
    property string errorMessage: ""

    // Timer to clear error message
    Timer {
        id: errorMessageTimer
        interval: 3000 // 3 seconds
        onTriggered: {
            errorMessage = ""
        }
    }

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

            Item {
                anchors {
                    left: parent.left
                    right: parent.right
                    verticalCenter: parent.verticalCenter
                }
                height: backButton.height

                // Back Button
                Button {
                    id: backButton
                    width: 40
                    height: 40
                    anchors {
                        left: parent.left
                        leftMargin: 16
                        verticalCenter: parent.verticalCenter
                    }
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
                    anchors {
                        left: backButton.right
                        leftMargin: 16
                        verticalCenter: parent.verticalCenter
                    }
                }

                // Generate Schedules Button - Only visible when at least 1 course is selected
                Button {
                    id: generateButton
                    width: 180
                    height: 40
                    anchors {
                        right: parent.right
                        rightMargin: 16
                        verticalCenter: parent.verticalCenter
                    }
                    visible: selectedCoursesRepeater.count > 0
                    enabled: selectedCoursesRepeater.count > 0

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

                Row {
                    anchors {
                        left: parent.left
                        top: parent.top
                        right: parent.right
                        margins: 12
                    }
                    spacing: 8

                    Column {
                        width: parent.width - courseCounter.width - 8
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
                                    id: courseRect
                                    width: courseContainer.width + 24
                                    height: 36
                                    radius: 4
                                    color: courseMouseArea.containsMouse ? "#ef4444" : "#4f46e5"  // Red when hovering, default purple/indigo

                                    // Container for the text and X symbol
                                    Row {
                                        id: courseContainer
                                        anchors.centerIn: parent
                                        spacing: 8

                                        Label {
                                            id: courseIdText
                                            text: courseId
                                            font.bold: true
                                            color: "#ffffff"
                                            anchors.verticalCenter: parent.verticalCenter
                                        }

                                        // X symbol - only visible on hover
                                        Text {
                                            id: removeIcon
                                            text: "✕"
                                            font.pixelSize: 14
                                            color: "#ffffff"
                                            visible: courseMouseArea.containsMouse
                                            anchors.verticalCenter: parent.verticalCenter
                                        }
                                    }

                                    MouseArea {
                                        id: courseMouseArea
                                        anchors.fill: parent
                                        hoverEnabled: true
                                        onClicked: {
                                            courseSelectionController.deselectCourse(index)
                                        }
                                        // Add cursor change on hover to indicate it's clickable
                                        cursorShape: Qt.PointingHandCursor
                                    }
                                }
                            }
                        }
                    }

                    // Course Counter
                    Rectangle {
                        id: courseCounter
                        width: 70
                        height: 70
                        radius: 8
                        color: "#f3f4f6"
                        border.color: "#d1d5db"

                        Column {
                            anchors.centerIn: parent
                            spacing: 4

                            Label {
                                anchors.horizontalCenter: parent.horizontalCenter
                                text: selectedCoursesRepeater.count + "/7"
                                font.pixelSize: 20
                                font.bold: true
                                color: "#1f2937"
                            }

                            Label {
                                anchors.horizontalCenter: parent.horizontalCenter
                                text: "Courses"
                                font.pixelSize: 12
                                color: "#6b7280"
                            }
                        }
                    }
                }
            }

            // Error message
            Rectangle {
                id: errorMessageContainer
                anchors {
                    top: selectedCoursesContainer.bottom
                    left: parent.left
                    right: parent.right
                    margins: 16
                }
                height: errorMessage === "" ? 0 : 40
                visible: errorMessage !== ""
                color: "#fef2f2"
                radius: 4
                border.color: "#fecaca"

                Label {
                    anchors.centerIn: parent
                    text: errorMessage
                    color: "#dc2626"
                    font.pixelSize: 14
                }
            }

            Label {
                id: courseListTitle
                anchors {
                    top: errorMessageContainer.visible ? errorMessageContainer.bottom : selectedCoursesContainer.bottom
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
                    width: parent.width - 30
                    height: 80
                    color: {
                        if (courseSelectionController.isCourseSelected(index)) {
                            return "#f0f9ff" // Selected course
                        } else if (selectedCoursesRepeater.count >= 7) {
                            return "#e5e7eb" // Disabled (max courses selected)
                        } else {
                            return "#ffffff" // Normal state
                        }
                    }
                    radius: 8
                    border.color: {
                        if (courseSelectionController.isCourseSelected(index)) {
                            return "#3b82f6" // Selected course
                        } else if (selectedCoursesRepeater.count >= 7) {
                            return "#d1d5db" // Disabled (max courses selected)
                        } else {
                            return "#e5e7eb" // Normal state
                        }
                    }
                    opacity: selectedCoursesRepeater.count >= 7 && !courseSelectionController.isCourseSelected(index) ? 0.7 : 1

                    // Using a connection to force update when selection changes
                    Connections {
                        target: courseSelectionController
                        function onSelectionChanged() {
                            // Update color based on selection state and max courses limit
                            if (courseSelectionController.isCourseSelected(index)) {
                                courseDelegate.color = "#f0f9ff" // Selected course
                                courseDelegate.border.color = "#3b82f6"
                                courseDelegate.opacity = 1
                            } else if (selectedCoursesRepeater.count >= 7) {
                                courseDelegate.color = "#e5e7eb" // Disabled (max courses selected)
                                courseDelegate.border.color = "#d1d5db"
                                courseDelegate.opacity = 0.7
                            } else {
                                courseDelegate.color = "#ffffff" // Normal state
                                courseDelegate.border.color = "#e5e7eb"
                                courseDelegate.opacity = 1
                            }
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
                            color: {
                                if (courseSelectionController.isCourseSelected(index)) {
                                    return "#dbeafe" // Selected
                                } else if (selectedCoursesRepeater.count >= 7) {
                                    return "#e5e7eb" // Disabled
                                } else {
                                    return "#f3f4f6" // Normal
                                }
                            }
                            radius: 4

                            Label {
                                anchors.centerIn: parent
                                text: courseId
                                font.bold: true
                                color: {
                                    if (courseSelectionController.isCourseSelected(index)) {
                                        return "#2563eb" // Selected
                                    } else if (selectedCoursesRepeater.count >= 7) {
                                        return "#9ca3af" // Disabled
                                    } else {
                                        return "#4b5563" // Normal
                                    }
                                }
                            }

                            // Update color when selection changes
                            Connections {
                                target: courseSelectionController
                                function onSelectionChanged() {
                                    // Update course ID box styling based on selection state and max courses limit
                                    if (courseSelectionController.isCourseSelected(index)) {
                                        courseIdBox.color = "#dbeafe" // Selected
                                        courseIdBox.children[0].color = "#2563eb"
                                    } else if (selectedCoursesRepeater.count >= 7) {
                                        courseIdBox.color = "#e5e7eb" // Disabled
                                        courseIdBox.children[0].color = "#9ca3af"
                                    } else {
                                        courseIdBox.color = "#f3f4f6" // Normal
                                        courseIdBox.children[0].color = "#4b5563"
                                    }
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
                                color: {
                                    if (selectedCoursesRepeater.count >= 7 && !courseSelectionController.isCourseSelected(index)) {
                                        return "#9ca3af" // Disabled
                                    } else {
                                        return "#1f2937" // Normal
                                    }
                                }
                            }

                            Label {
                                text: "Instructor: " + teacherName
                                font.pixelSize: 14
                                color: {
                                    if (selectedCoursesRepeater.count >= 7 && !courseSelectionController.isCourseSelected(index)) {
                                        return "#9ca3af" // Disabled
                                    } else {
                                        return "#6b7280" // Normal
                                    }
                                }
                            }
                        }
                    }

                    MouseArea {
                        anchors.fill: parent
                        onClicked: {
                            if (selectedCoursesRepeater.count >= 7 && !courseSelectionController.isCourseSelected(index)) {
                                // Show error message if trying to select more than 7 courses
                                errorMessage = "You have selected the maximum of 7 courses"
                                errorMessageTimer.restart()
                            } else {
                                courseSelectionController.toggleCourseSelection(index)
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
                text: "© 2025 Schedulify. All rights reserved."
                color: "#6b7280"
                font.pixelSize: 12
            }
        }
    }
}
