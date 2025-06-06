import QtQuick 2.15
import QtQuick.Layouts 1.15
import QtQuick.Controls 2.15
import QtQuick.Controls.Basic
import "."

Page {
    id: courseListScreen

    Connections {
        target: courseSelectionController

        function onErrorMessage(message) {
            showErrorMessage(message);
        }
    }

    function showErrorMessage(msg) {
        errorDialogText = msg;
        errorDialog.open();
    }

    // Property to store error message
    property string errorMessage: ""
    // Property to store search text
    property string searchText: ""

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
                height: coursesBackButton.height

                // Back Button
                Button {
                    id: coursesBackButton
                    width: 40
                    height: 40
                    anchors {
                        left: parent.left
                        leftMargin: 16
                        verticalCenter: parent.verticalCenter
                    }
                    background: Rectangle {
                        color: coursesBackMouseArea.containsMouse ? "#a8a8a8" : "#f3f4f6"
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
                        id: coursesBackMouseArea
                        anchors.fill: parent
                        hoverEnabled: true
                        onClicked: courseSelectionController.goBack()
                        cursorShape: Qt.PointingHandCursor
                    }
                }

                // Screen Title
                Label {
                    id: titleLabel
                    text: "Available Courses"
                    font.pixelSize: 20
                    color: "#1f2937"
                    anchors {
                        left: coursesBackButton.right
                        leftMargin: 16
                        verticalCenter: parent.verticalCenter
                    }
                }

                Button {
                    id: logButtonB
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
                                    var logWindow = component.createObject(courseListScreen, {
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

                Button {
                    id: generateButton
                    width: 180
                    height: 40
                    anchors {
                        right: parent.right
                        rightMargin: 25 + logButtonB.width
                        verticalCenter: parent.verticalCenter
                    }
                    visible: selectedCoursesRepeater.count > 0
                    enabled: selectedCoursesRepeater.count > 0

                    background: Rectangle {
                        color: generateMouseArea.containsMouse ? "#35455c" : "#1f2937"
                        radius: 4
                        implicitWidth: 180
                        implicitHeight: 40
                    }
                    font.bold: true
                    contentItem: Text {
                        text: "Generate Schedules →"
                        color: "white"
                        font.pixelSize: 14
                        horizontalAlignment: Text.AlignHCenter
                        verticalAlignment: Text.AlignVCenter
                    }

                    MouseArea {
                        id: generateMouseArea
                        anchors.fill: parent
                        hoverEnabled: true
                        onClicked: courseSelectionController.generateSchedules()
                        cursorShape: Qt.PointingHandCursor
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
                                model: courseSelectionController ? courseSelectionController.selectedCoursesModel : null

                                Rectangle {
                                    id: courseRect
                                    width: courseContainer.width + 30
                                    height: 36
                                    radius: 4
                                    color: courseMouseArea.containsMouse ? "#ef4444" : "#4f46e5"

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
                                    }

                                    MouseArea {
                                        id: courseMouseArea
                                        anchors.fill: parent
                                        hoverEnabled: true
                                        onClicked: courseSelectionController.deselectCourse(index)
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

            Row {
                id: courseListHeader
                anchors {
                    top: courseListTitle.bottom
                    left: parent.left
                    right: parent.right
                    margins: 16
                }
                spacing: 16
                height: 60

                // Search bar
                Rectangle {
                    id: searchBar
                    width: parent.width
                    height: 50
                    radius: 8
                    color: "#ffffff"
                    border.color: "#e5e7eb"

                    Row {
                        anchors {
                            left: parent.left
                            right: parent.right
                            verticalCenter: parent.verticalCenter
                            leftMargin: 16
                            rightMargin: 16
                        }
                        spacing: 8

                        // Search icon

                        // Search input field
                        TextField {
                            id: searchField
                            width: parent.width - 32
                            placeholderText: "Search by course ID, name, or instructor..."
                            placeholderTextColor: "#9CA3AF"
                            font.pixelSize: 14
                            color: "#1f2937"
                            selectByMouse: true

                            background: Rectangle {
                                radius: 4
                                color: "#FFFFFF"
                            }

                            onTextChanged: {
                                searchText = text
                                courseSelectionController.filterCourses(text)
                            }
                        }

                        // Clear icon
                        Button {
                            id: clearSearch
                            visible: searchField.text !== ""
                            width: 30
                            height: 24
                            anchors.verticalCenter: parent.verticalCenter

                            background: Rectangle {
                                color: "transparent"
                            }

                            contentItem: Text {
                                text: "✕"
                                font.pixelSize: 18
                                font.bold: true
                                color: clearSearchMouseArea.containsMouse ? "#d81a1a" : "#000000"
                                horizontalAlignment: Text.AlignHCenter
                                verticalAlignment: Text.AlignVCenter
                            }

                            MouseArea {
                                id: clearSearchMouseArea
                                anchors.fill: parent
                                hoverEnabled: true
                                onClicked: {
                                    searchField.text = ""
                                    searchText = ""
                                    courseSelectionController.resetFilter()
                                }
                                cursorShape: Qt.PointingHandCursor
                            }

                            onClicked: {
                                searchField.text = ""
                                searchText = ""
                                courseSelectionController.resetFilter()
                            }
                        }
                    }
                }
            }

            Label {
                id: courseListDescription
                anchors {
                    top: courseListHeader.bottom
                    left: parent.left
                    right: parent.right
                    leftMargin: 16
                    rightMargin: 16
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
                model: courseSelectionController ? courseSelectionController.filteredCourseModel : null
                spacing: 8

                // Delegate for course items
                delegate: Rectangle {
                    id: courseDelegate
                    width: courseListView ? courseListView.width : 120
                    height: 80
                    color: {
                        if (courseSelectionController.isCourseSelected(originalIndex)) {
                            return "#f0f9ff" // Selected course
                        } else if (selectedCoursesRepeater.count >= 7) {
                            return "#e5e7eb" // Disabled (max courses selected)
                        } else {
                            return "#ffffff" // Normal state
                        }
                    }
                    radius: 8
                    border.color: {
                        if (courseSelectionController.isCourseSelected(originalIndex)) {
                            return "#3b82f6" // Selected course
                        } else if (selectedCoursesRepeater.count >= 7) {
                            return "#d1d5db" // Disabled (max courses selected)
                        } else {
                            return "#e5e7eb" // Normal state
                        }
                    }
                    opacity: selectedCoursesRepeater.count >= 7 && !courseSelectionController.isCourseSelected(originalIndex) ? 0.7 : 1

                    Connections {
                        target: courseSelectionController
                        function onSelectionChanged() {
                            // Update color based on selection state and max courses limit
                            if (courseSelectionController.isCourseSelected(originalIndex)) {
                                courseDelegate.color = "#f0f9ff"
                                courseDelegate.border.color = "#3b82f6"
                                courseDelegate.opacity = 1
                                courseIdBox.color = "#dbeafe"
                                courseIdBoxLabel.color = "#2563eb"
                            } else if (selectedCoursesRepeater.count >= 7) {
                                courseDelegate.color = "#e5e7eb"
                                courseDelegate.border.color = "#d1d5db"
                                courseDelegate.opacity = 0.7
                                courseIdBox.color = "#e5e7eb"
                                courseIdBoxLabel.color = "#9ca3af"
                            } else {
                                courseDelegate.color = "#ffffff"
                                courseDelegate.border.color = "#e5e7eb"
                                courseDelegate.opacity = 1
                                courseIdBox.color = "#f3f4f6"
                                courseIdBoxLabel.color = "#4b5563"
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
                                if (courseSelectionController.isCourseSelected(originalIndex)) {
                                    return "#dbeafe" // Selected
                                } else if (selectedCoursesRepeater.count >= 7) {
                                    return "#e5e7eb" // Disabled
                                } else {
                                    return "#f3f4f6" // Normal
                                }
                            }
                            radius: 4

                            Label {
                                id: courseIdBoxLabel
                                anchors.centerIn: parent
                                text: courseId
                                font.bold: true
                                color: {
                                    if (courseSelectionController.isCourseSelected(originalIndex)) {
                                        return "#2563eb" // Selected
                                    } else if (selectedCoursesRepeater.count >= 7) {
                                        return "#9ca3af" // Disabled
                                    } else {
                                        return "#4b5563" // Normal
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
                                    if (selectedCoursesRepeater.count >= 7 && !courseSelectionController.isCourseSelected(originalIndex)) {
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
                                    if (selectedCoursesRepeater.count >= 7 && !courseSelectionController.isCourseSelected(originalIndex)) {
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
                            if (selectedCoursesRepeater.count >= 7 && !courseSelectionController.isCourseSelected(originalIndex)) {
                                // Show error message if trying to select more than 7 courses
                                errorMessage = "You have selected the maximum of 7 courses"
                                errorMessageTimer.restart()
                            } else {
                                courseSelectionController.toggleCourseSelection(originalIndex)
                            }
                        }
                    }
                }

                // Empty state message when no courses match the search
                Item {
                    anchors.centerIn: parent
                    width: parent.width * 0.8
                    height: 100
                    visible: courseListView.count === 0

                    Column {
                        anchors.centerIn: parent
                        spacing: 8

                        Text {
                            anchors.horizontalCenter: parent.horizontalCenter
                            text: "🔍"
                            font.pixelSize: 24
                        }

                        Text {
                            anchors.horizontalCenter: parent.horizontalCenter
                            text: "No courses found"
                            font.pixelSize: 18
                            font.bold: true
                            color: "#4b5563"
                        }

                        Text {
                            anchors.horizontalCenter: parent.horizontalCenter
                            text: "Try a different search term"
                            font.pixelSize: 14
                            color: "#6b7280"
                            horizontalAlignment: Text.AlignHCenter
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
