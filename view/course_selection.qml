import QtQuick 2.15
import QtQuick.Layouts 1.15
import QtQuick.Controls 2.15
import QtQuick.Controls.Basic
import "popups"
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

    // Add Block Time Popup
    SlotBlockMenu {
        id: timeBlockPopup
        parent: Overlay.overlay

        onBlockTimeAdded: function(day, startTime, endTime) {
            courseSelectionController.addBlockTime(day, startTime, endTime);
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
                        leftMargin: 15
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

                // Page Title
                Label {
                    id: titleLabel
                    text: "Course Selection & Schedule Preferences"
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
                        color: logMouseArea.containsMouse ? "#a8a8a8" : "#f3f4f6"
                        radius: 10
                    }

                    contentItem: Item {
                        anchors.fill: parent

                        Image {
                            id: logIcon
                            anchors.centerIn: parent
                            width: 24
                            height: 24
                            source: "qrc:/icons/ic-logs.svg"
                            sourceSize.width: 22
                            sourceSize.height: 22
                        }

                        ToolTip {
                            id: logsTooltip
                            text: "Open Application Logs"
                            visible: logMouseArea.containsMouse
                            delay: 500
                            timeout: 3000

                            background: Rectangle {
                                color: "#374151"
                                radius: 4
                                border.color: "#4b5563"
                            }

                            contentItem: Text {
                                text: logsTooltip.text
                                color: "white"
                                font.pixelSize: 12
                            }
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
                }

                Button {
                    id: generateButton
                    width: 180
                    height: 40
                    anchors {
                        right: logButtonB.left
                        rightMargin: 10
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

        // Main content area with horizontal split
        Row {
            id: mainContent
            anchors {
                top: header.bottom
                left: parent.left
                right: parent.right
                bottom: footer.top
                margins: 16
            }
            spacing: 16

            // Left side - Course List (2/3 of width)
            Rectangle {
                id: courseListArea
                width: (parent.width * 2 / 3) - 8
                height: parent.height
                color: "#ffffff"
                radius: 8
                border.color: "#e5e7eb"

                Column {
                    anchors.fill: parent
                    anchors.margins: 16
                    spacing: 16

                    // Error message
                    Rectangle {
                        id: errorMessageContainer
                        width: parent.width
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
                        text: "Available Courses"
                        font.pixelSize: 24
                        color: "#1f2937"
                    }

                    // Search bar
                    Rectangle {
                        id: searchBar
                        width: parent.width
                        height: 50
                        radius: 8
                        color: "#f9fafb"
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

                            TextField {
                                id: searchField
                                width: parent.width - clearSearch.width - 8
                                placeholderText: "Search by course ID, name, or instructor..."
                                placeholderTextColor: "#9CA3AF"
                                font.pixelSize: 14
                                color: "#1f2937"
                                selectByMouse: true

                                background: Rectangle {
                                    radius: 4
                                    color: "#FFFFFF"
                                    border.color: "#e5e7eb"
                                }

                                onTextChanged: {
                                    searchText = text
                                    courseSelectionController.filterCourses(text)
                                }
                            }

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
                            }
                        }
                    }

                    Label {
                        text: "Click on a course to select it for your schedule"
                        color: "#6b7280"
                    }

                    // ListView for courses
                    ListView {
                        id: courseListView
                        width: parent.width
                        height: parent.height - courseListTitle.height - searchBar.height - 100
                        clip: true
                        model: courseSelectionController ? courseSelectionController.filteredCourseModel : null
                        spacing: 8

                        delegate: Rectangle {
                            id: courseDelegate
                            width: courseListView.width
                            height: 80
                            color: {
                                if (courseSelectionController.isCourseSelected(originalIndex)) {
                                    return "#f0f9ff"
                                } else if (selectedCoursesRepeater.count >= 7) {
                                    return "#e5e7eb"
                                } else {
                                    return "#ffffff"
                                }
                            }
                            radius: 8
                            border.color: {
                                if (courseSelectionController.isCourseSelected(originalIndex)) {
                                    return "#3b82f6"
                                } else if (selectedCoursesRepeater.count >= 7) {
                                    return "#d1d5db"
                                } else {
                                    return "#e5e7eb"
                                }
                            }
                            opacity: selectedCoursesRepeater.count >= 7 && !courseSelectionController.isCourseSelected(originalIndex) ? 0.7 : 1

                            Connections {
                                target: courseSelectionController
                                function onSelectionChanged() {
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

                                Rectangle {
                                    id: courseIdBox
                                    Layout.preferredWidth: 80
                                    Layout.preferredHeight: 56
                                    color: {
                                        if (courseSelectionController.isCourseSelected(originalIndex)) {
                                            return "#dbeafe"
                                        } else if (selectedCoursesRepeater.count >= 7) {
                                            return "#e5e7eb"
                                        } else {
                                            return "#f3f4f6"
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
                                                return "#2563eb"
                                            } else if (selectedCoursesRepeater.count >= 7) {
                                                return "#9ca3af"
                                            } else {
                                                return "#4b5563"
                                            }
                                        }
                                    }
                                }

                                Column {
                                    Layout.fillWidth: true
                                    spacing: 4

                                    Label {
                                        text: courseName
                                        font.pixelSize: 16
                                        font.bold: true
                                        color: {
                                            if (selectedCoursesRepeater.count >= 7 && !courseSelectionController.isCourseSelected(originalIndex)) {
                                                return "#9ca3af"
                                            } else {
                                                return "#1f2937"
                                            }
                                        }
                                    }

                                    Label {
                                        text: "Instructor: " + teacherName
                                        font.pixelSize: 14
                                        color: {
                                            if (selectedCoursesRepeater.count >= 7 && !courseSelectionController.isCourseSelected(originalIndex)) {
                                                return "#9ca3af"
                                            } else {
                                                return "#6b7280"
                                            }
                                        }
                                    }
                                }
                            }

                            MouseArea {
                                anchors.fill: parent
                                onClicked: {
                                    if (selectedCoursesRepeater.count >= 7 && !courseSelectionController.isCourseSelected(originalIndex)) {
                                        errorMessage = "You have selected the maximum of 7 courses"
                                        errorMessageTimer.restart()
                                    } else {
                                        courseSelectionController.toggleCourseSelection(originalIndex)
                                    }
                                }
                            }
                        }

                        // Empty state message
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

                        ScrollBar.vertical: ScrollBar {
                            active: true
                        }
                    }
                }
            }

            // Right side - Selected Courses and Block Times (1/3 of width)
            Column {
                id: rightPanel
                width: (parent.width * 1 / 3) - 8
                height: parent.height
                spacing: 16

                // Selected Courses Section
                Rectangle {
                    id: selectedCoursesSection
                    width: parent.width
                    height: (parent.height - 16) / 2
                    color: "#ffffff"
                    radius: 8
                    border.color: "#e5e7eb"

                    Column {
                        anchors.fill: parent
                        anchors.margins: 16
                        spacing: 12

                        Row {
                            width: parent.width
                            spacing: 8

                            Label {
                                text: "Selected Courses"
                                font.pixelSize: 18
                                font.bold: true
                                color: "#1f2937"
                                anchors.verticalCenter: parent.verticalCenter
                            }

                            Rectangle {
                                id: courseCounter
                                width: 60
                                height: 30
                                radius: 4
                                color: "#f3f4f6"
                                border.color: "#d1d5db"
                                anchors.verticalCenter: parent.verticalCenter

                                Label {
                                    anchors.centerIn: parent
                                    text: selectedCoursesRepeater.count + "/7"
                                    font.pixelSize: 14
                                    font.bold: true
                                    color: "#1f2937"
                                }
                            }
                        }

                        ScrollView {
                            width: parent.width
                            height: parent.height - 50
                            clip: true
                            ScrollBar.horizontal.policy: ScrollBar.AlwaysOff

                            Column {
                                id: selectedCoursesColumn
                                width: parent.width
                                spacing: 8

                                Repeater {
                                    id: selectedCoursesRepeater
                                    model: courseSelectionController ? courseSelectionController.selectedCoursesModel : null

                                    Rectangle {
                                        width: selectedCoursesColumn.width  // Reference parent column width
                                        height: 50
                                        radius: 6
                                        color: "#f0f9ff"
                                        border.color: "#3b82f6"

                                        Row {
                                            anchors {
                                                left: parent.left
                                                right: parent.right
                                                verticalCenter: parent.verticalCenter
                                                margins: 12
                                            }
                                            spacing: 8

                                            Rectangle {
                                                width: 40
                                                height: 26
                                                radius: 4
                                                color: "#dbeafe"

                                                Label {
                                                    anchors.centerIn: parent
                                                    text: courseId
                                                    font.bold: true
                                                    font.pixelSize: 12
                                                    color: "#2563eb"
                                                }
                                            }

                                            Column {
                                                anchors.verticalCenter: parent.verticalCenter
                                                spacing: 2
                                                width: Math.max(100, parent.parent.width - 80)  // Explicit width calculation

                                                Label {
                                                    text: courseName
                                                    font.pixelSize: 14
                                                    font.bold: true
                                                    color: "#1f2937"
                                                    width: parent.width
                                                    elide: Text.ElideRight
                                                }

                                                Label {
                                                    text: teacherName
                                                    font.pixelSize: 12
                                                    color: "#6b7280"
                                                    width: parent.width
                                                    elide: Text.ElideRight
                                                }
                                            }

                                            Item {
                                                width: 8  // Spacer
                                            }

                                            Button {
                                                width: 24
                                                height: 24
                                                anchors.verticalCenter: parent.verticalCenter

                                                background: Rectangle {
                                                    color: removeMouseArea.containsMouse ? "#ef4444" : "#f87171"
                                                    radius: 12
                                                }

                                                contentItem: Text {
                                                    text: "×"
                                                    color: "white"
                                                    font.pixelSize: 16
                                                    font.bold: true
                                                    horizontalAlignment: Text.AlignHCenter
                                                    verticalAlignment: Text.AlignVCenter
                                                }

                                                MouseArea {
                                                    id: removeMouseArea
                                                    anchors.fill: parent
                                                    hoverEnabled: true
                                                    onClicked: courseSelectionController.deselectCourse(index)
                                                    cursorShape: Qt.PointingHandCursor
                                                }
                                            }
                                        }
                                    }
                                }
                            }
                        }
                    }
                }

                // Block Times Section
                Rectangle {
                    id: blockTimesSection
                    width: parent.width
                    height: (parent.height - 16) / 2
                    color: "#ffffff"
                    radius: 8
                    border.color: "#e5e7eb"

                    Column {
                        anchors.fill: parent
                        anchors.margins: 16
                        spacing: 12

                        Row {
                            width: parent.width
                            spacing: 8

                            Label {
                                text: "Block Times"
                                font.pixelSize: 18
                                font.bold: true
                                color: "#1f2937"
                                anchors.verticalCenter: parent.verticalCenter
                            }

                            Item {
                                Layout.fillWidth: true
                                width: parent.parent.width - addBlockButton.width - 150
                            }

                            Button {
                                id: addBlockButton
                                width: 120
                                height: 32
                                anchors.verticalCenter: parent.verticalCenter

                                background: Rectangle {
                                    color: addBlockMouseArea.containsMouse ? "#35455c" : "#1f2937"
                                    radius: 4
                                }

                                contentItem: Text {
                                    text: "+ Add Block"
                                    color: "white"
                                    font.pixelSize: 12
                                    font.bold: true
                                    horizontalAlignment: Text.AlignHCenter
                                    verticalAlignment: Text.AlignVCenter
                                }

                                MouseArea {
                                    id: addBlockMouseArea
                                    anchors.fill: parent
                                    hoverEnabled: true
                                    onClicked: timeBlockPopup.open()
                                    cursorShape: Qt.PointingHandCursor
                                }
                            }
                        }

                        ScrollView {
                            width: parent.width
                            height: parent.height - 50
                            clip: true
                            ScrollBar.horizontal.policy: ScrollBar.AlwaysOff

                            Column {
                                width: parent.width
                                spacing: 8

                                Repeater {
                                    id: blockTimesRepeater
                                    model: courseSelectionController ? courseSelectionController.blocksModel : null

                                    Rectangle {
                                        width: parent ? parent.width : 200  // Ensure explicit width
                                        height: 60
                                        radius: 6
                                        color: "#fef3c7"
                                        border.color: "#f59e0b"

                                        Row {
                                            anchors {
                                                left: parent.left
                                                right: parent.right
                                                verticalCenter: parent.verticalCenter
                                                margins: 12
                                            }
                                            spacing: 8

                                            Column {
                                                anchors.verticalCenter: parent.verticalCenter
                                                spacing: 4
                                                width: Math.max(100, parent.parent.width - 40)  // Explicit width calculation

                                                Label {
                                                    text: teacherName // Day (moved from courseName to teacherName)
                                                    font.pixelSize: 14
                                                    font.bold: true
                                                    color: "#92400e"
                                                    width: parent.width
                                                    elide: Text.ElideRight
                                                }

                                                Label {
                                                    text: courseId // Time range
                                                    font.pixelSize: 12
                                                    color: "#a16207"
                                                    width: parent.width
                                                    elide: Text.ElideRight
                                                }
                                            }

                                            Item {
                                                width: 8  // Spacer
                                            }

                                            Button {
                                                width: 24
                                                height: 24
                                                anchors.verticalCenter: parent.verticalCenter

                                                background: Rectangle {
                                                    color: removeBlockMouseArea.containsMouse ? "#dc2626" : "#ef4444"
                                                    radius: 12
                                                }

                                                contentItem: Text {
                                                    text: "×"
                                                    color: "white"
                                                    font.pixelSize: 16
                                                    font.bold: true
                                                    horizontalAlignment: Text.AlignHCenter
                                                    verticalAlignment: Text.AlignVCenter
                                                }

                                                MouseArea {
                                                    id: removeBlockMouseArea
                                                    anchors.fill: parent
                                                    hoverEnabled: true
                                                    onClicked: courseSelectionController.removeBlockTime(index)
                                                    cursorShape: Qt.PointingHandCursor
                                                }
                                            }
                                        }
                                    }
                                }

                                // Empty state for block times
                                Item {
                                    width: parent.width
                                    height: 80
                                    visible: blockTimesRepeater.count === 0

                                    Column {
                                        anchors.centerIn: parent
                                        spacing: 8

                                        Text {
                                            anchors.horizontalCenter: parent.horizontalCenter
                                            text: "🚫"
                                            font.pixelSize: 20
                                        }

                                        Text {
                                            anchors.horizontalCenter: parent.horizontalCenter
                                            text: "No blocked times"
                                            font.pixelSize: 14
                                            color: "#6b7280"
                                            horizontalAlignment: Text.AlignHCenter
                                        }

                                        Text {
                                            anchors.horizontalCenter: parent.horizontalCenter
                                            text: "Add time slots you want to avoid"
                                            font.pixelSize: 12
                                            color: "#9ca3af"
                                            horizontalAlignment: Text.AlignHCenter
                                        }
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }

        // Footer
        Rectangle {
            id: footer
            width: parent.width
            height: 30
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