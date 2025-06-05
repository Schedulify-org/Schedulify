import QtQuick 2.15
import QtQuick.Layouts 1.15
import QtQuick.Controls 2.15
import QtQuick.Controls.Basic
import "popups"
import "."

Page {
    id: courseListScreen

    // Add properties for validation state
    property bool validationInProgress: courseSelectionController ? courseSelectionController.validationInProgress : false
    property var validationErrors: courseSelectionController ? courseSelectionController.validationErrors : []
    property bool validationExpanded: false

    // Add property to track our log window
    property var logWindow: null

    // Clean up log window when this page is destroyed
    Component.onDestruction: {
        if (logWindow) {
            logWindow.close();
            logWindow = null;
            if (logDisplayController) {
                logDisplayController.setLogWindowOpen(false);
            }
        }
    }

    Connections {
        target: courseSelectionController

        function onErrorMessage(message) {
            showErrorMessage(message);
        }

        function onValidationStateChanged() {
            // Update local properties when validation state changes
            if (courseSelectionController) {
                validationInProgress = courseSelectionController.validationInProgress;
                validationErrors = courseSelectionController.validationErrors || [];

                // Auto-collapse when validation starts
                if (validationInProgress) {
                    validationExpanded = false;
                }
            }
        }
    }

    function showErrorMessage(msg) {
        errorDialogText = msg;
        errorDialog.open();
    }

    property string errorMessage: ""
    property string searchText: ""

    Timer {
        id: errorMessageTimer
        interval: 3000 // 3 seconds
        onTriggered: {
            errorMessage = ""
        }
    }

    SlotBlockMenu {
        id: timeBlockPopup
        parent: Overlay.overlay

        onBlockTimeAdded: function(day, startTime, endTime) {
            courseSelectionController.addBlockTime(day, startTime, endTime);
        }
    }

    AddCoursePopup {
        id: addCoursePopup
        parent: Overlay.overlay

        onCourseCreated: function(courseName, courseId, teacherName, sessionGroups) {
            courseSelectionController.createNewCourse(courseName, courseId, teacherName, sessionGroups);
        }
    }

    Rectangle {
        id: root
        anchors.fill: parent
        color: "#f9fafb"

        // Header
        Rectangle {
            id: header
            anchors {
                top: parent.top
                left: parent.left
                right: parent.right
            }
            height: 80
            color: "#ffffff"
            border.color: "#e5e7eb"

            Item {
                id: headerContent
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
                        onClicked: {
                            // Clean up log window before navigating back
                            if (logWindow) {
                                logWindow.close();
                                logWindow = null;
                                if (logDisplayController) {
                                    logDisplayController.setLogWindowOpen(false);
                                }
                            }
                            courseSelectionController.goBack();
                        }
                        cursorShape: Qt.PointingHandCursor
                    }
                }

                // Page Title
                Label {
                    id: titleLabel
                    text: "Course Selection"
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
                            // Check if we already have a log window open
                            if (logWindow && logWindow.visible) {
                                // Bring existing window to front
                                logWindow.raise();
                                logWindow.requestActivate();
                                return;
                            }

                            // Create new log window if none exists or if controller says none is open
                            if (!logDisplayController.isLogWindowOpen || !logWindow) {
                                var component = Qt.createComponent("qrc:/log_display.qml");
                                if (component.status === Component.Ready) {
                                    logDisplayController.setLogWindowOpen(true);
                                    logWindow = component.createObject(null, { // Create as independent window
                                        "onClosing": function(close) {
                                            logDisplayController.setLogWindowOpen(false);
                                            logWindow = null; // Clear our reference
                                        }
                                    });

                                    if (logWindow) {
                                        logWindow.show();
                                    }
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

        // Validation Status Row
        Rectangle {
            id: validationStatusRow
            anchors {
                top: header.bottom
                topMargin: 16
                left: parent.left
                right: parent.right
                leftMargin: 16
                rightMargin: 16
            }
            height: validationExpanded ? 208 : 60
            radius: 8
            color: {
                if (validationInProgress) return "#fff6dc"
                if (validationErrors.length === 0) return "#e1fff1"
                return "#ffecec"
            }
            border.color: {
                if (validationInProgress) return "#f59e0b"
                if (validationErrors.length === 0) return "#10b981"
                return "#f56565"
            }
            border.width: 1

            Behavior on height {
                NumberAnimation {
                    duration: 200
                    easing.type: Easing.OutCubic
                }
            }

            Column {
                id: validationContent
                anchors {
                    left: parent.left
                    right: parent.right
                    top: parent.top
                    margins: 16
                }
                spacing: 12

                Row {
                    anchors {
                        left: parent.left
                        right: parent.right
                    }
                    height: 28
                    spacing: 12

                    // Status Icon
                    Image {
                        id: statusIcon
                        anchors.verticalCenter: parent.verticalCenter
                        width: 30
                        height: 30
                        source: {
                            if (validationInProgress) return "qrc:/icons/ic-loading.svg"
                            if (validationErrors.length === 0) return "qrc:/icons/ic-valid.svg"
                            return "qrc:/icons/ic-warning.svg"
                        }
                        sourceSize.width: 30
                        sourceSize.height: 30
                    }

                    Text {
                        anchors.verticalCenter: parent.verticalCenter
                        text: {
                            if (validationInProgress) return "Scanning courses..."
                            if (!validationErrors || validationErrors.length === 0) return "All courses parsed correctly"

                            // Count different types of messages
                            var parserWarnings = 0;
                            var parserErrors = 0;
                            var validationErrorCount = 0;

                            for (var i = 0; i < validationErrors.length; i++) {
                                var msg = validationErrors[i];
                                if (msg.includes("[Parser Warning]")) {
                                    parserWarnings++;
                                } else if (msg.includes("[Parser Error]")) {
                                    parserErrors++;
                                } else if (msg.includes("[Validation]")) {
                                    validationErrorCount++;
                                }
                            }

                            var totalIssues = parserWarnings + parserErrors + validationErrorCount;

                            if (parserErrors > 0 || validationErrorCount > 0) {
                                return "Found " + totalIssues + " issues in your course file"
                            } else {
                                return "Found " + parserWarnings + " warnings in your course file"
                            }
                        }
                        font.pixelSize: 14
                        font.bold: true
                        color: {
                            if (validationInProgress) return "#92400e"
                            if (!validationErrors || validationErrors.length === 0) return "#065f46"
                            return "#b91c1c"
                        }
                    }

                    Button {
                        anchors.verticalCenter: parent.verticalCenter
                        visible: validationErrors && validationErrors.length > 0
                        width: 24
                        height: 24
                        background: Rectangle {
                            color: "transparent"
                        }
                        contentItem: Text {
                            text: validationExpanded ? "▼" : "▶"
                            font.pixelSize: 12
                            color: "#b91c1c"
                            horizontalAlignment: Text.AlignHCenter
                            verticalAlignment: Text.AlignVCenter
                        }
                        onClicked: validationExpanded = !validationExpanded
                    }
                }

                Rectangle {
                    visible: validationExpanded && validationErrors && validationErrors.length > 0
                    anchors {
                        left: parent.left
                        right: parent.right
                    }
                    // Fixed height to accommodate approximately 2 rows of messages
                    height: visible ? 120 : 0
                    color: "#fef2f2"
                    radius: 6
                    border.color: "#fca5a5"
                    border.width: 1

                    ScrollView {
                        id: errorsList
                        anchors {
                            fill: parent
                            margins: 8
                        }
                        clip: true
                        ScrollBar.horizontal.policy: ScrollBar.AlwaysOff
                        ScrollBar.vertical.policy: ScrollBar.AsNeeded

                        Column {
                            id: messagesColumn
                            width: errorsList.width
                            spacing: 8

                            Repeater {
                                model: validationErrors
                                delegate: Rectangle {
                                    width: messagesColumn.width
                                    height: Math.max(40, errorText.contentHeight + 16) // Minimum height for consistency
                                    color: {
                                        var msg = modelData;
                                        if (msg.includes("[Parser Warning]")) return "#fffbeb"
                                        if (msg.includes("[Parser Error]")) return "#fef2f2"
                                        if (msg.includes("[Validation]")) return "#fef2f2"
                                        return "#ffffff"
                                    }
                                    radius: 4
                                    border.color: {
                                        var msg = modelData;
                                        if (msg.includes("[Parser Warning]")) return "#fbbf24"
                                        if (msg.includes("[Parser Error]")) return "#f87171"
                                        if (msg.includes("[Validation]")) return "#f87171"
                                        return "#e5e7eb"
                                    }
                                    border.width: 1

                                    Text {
                                        id: errorText
                                        anchors {
                                            left: parent.left
                                            right: parent.right
                                            verticalCenter: parent.verticalCenter
                                            leftMargin: 8
                                            rightMargin: 8
                                        }
                                        text: {
                                            // Clean up the message by removing prefixes for display
                                            var msg = modelData;
                                            if (msg.includes("[Parser Warning] ")) {
                                                return "⚠️ " + msg.replace("[Parser Warning] ", "");
                                            } else if (msg.includes("[Parser Error] ")) {
                                                return "❌ " + msg.replace("[Parser Error] ", "");
                                            } else if (msg.includes("[Validation] ")) {
                                                return "🔍 " + msg.replace("[Validation] ", "");
                                            }
                                            return msg;
                                        }
                                        font.pixelSize: 12
                                        color: {
                                            var msg = modelData;
                                            if (msg.includes("[Parser Warning]")) return "#92400e"
                                            return "#991b1b"
                                        }
                                        wrapMode: Text.WordWrap
                                        maximumLineCount: 2 // Limit to 2 lines per message for consistency
                                        elide: Text.ElideRight
                                    }
                                }
                            }
                        }
                    }
                }
            }

            MouseArea {
                anchors.fill: parent
                enabled: validationErrors && validationErrors.length > 0
                cursorShape: enabled ? Qt.PointingHandCursor : Qt.ArrowCursor
                onClicked: {
                    if (validationErrors && validationErrors.length > 0) {
                        validationExpanded = !validationExpanded
                    }
                }
            }
        }

        // Main content
        Item {
            id: mainContent
            anchors {
                top: validationStatusRow.bottom
                topMargin: 16
                left: parent.left
                right: parent.right
                bottom: footer.top
                margins: 16
            }

            // Left side
            Rectangle {
                id: courseListArea
                anchors {
                    top: parent.top
                    left: parent.left
                    bottom: parent.bottom
                }
                width: (parent.width * 2 / 3) - 8
                color: "#ffffff"
                radius: 8
                border.color: "#e5e7eb"

                Item {
                    id: courseListContent
                    anchors {
                        fill: parent
                        margins: 16
                    }

                    // Error message
                    Rectangle {
                        id: errorMessageContainer
                        anchors {
                            top: parent.top
                            left: parent.left
                            right: parent.right
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

                    // Header
                    RowLayout {
                        id: courseListTitleRow
                        anchors {
                            top: errorMessageContainer.bottom
                            topMargin: errorMessage === "" ? 0 : 16
                            left: parent.left
                            right: parent.right
                        }
                        height: 30

                        Label {
                            text: "Available Courses"
                            font.pixelSize: 24
                            color: "#1f2937"
                            Layout.fillWidth: true
                        }

                        Button {
                            id: createCourseButton
                            Layout.preferredWidth: 140
                            Layout.preferredHeight: 32

                            background: Rectangle {
                                color: createCourseMouseArea.containsMouse ? "#35455c" : "#1f2937"
                                radius: 4
                            }

                            Text {
                                anchors.centerIn: parent
                                text: "+ Create Course"
                                font.pixelSize: 12
                                font.bold: true
                                color: "white"
                            }

                            MouseArea {
                                id: createCourseMouseArea
                                anchors.fill: parent
                                hoverEnabled: true
                                onClicked: {
                                    addCoursePopup.open()
                                }
                                cursorShape: Qt.PointingHandCursor
                            }
                        }
                    }

                    // Search bar
                    Rectangle {
                        id: searchBar
                        anchors {
                            top: courseListTitleRow.bottom
                            topMargin: 16
                            left: parent.left
                            right: parent.right
                        }
                        height: 50
                        radius: 8
                        color: "#f9fafb"
                        border.color: "#e5e7eb"

                        Item {
                            id: searchContent
                            anchors {
                                left: parent.left
                                right: parent.right
                                verticalCenter: parent.verticalCenter
                                leftMargin: 16
                                rightMargin: 16
                            }
                            height: parent.height

                            TextField {
                                id: searchField
                                anchors {
                                    left: parent.left
                                    right: clearSearch.left
                                    rightMargin: 8
                                    verticalCenter: parent.verticalCenter
                                }
                                height: 30
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
                                anchors {
                                    right: parent.right
                                    verticalCenter: parent.verticalCenter
                                }

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
                        id: searchHelpText
                        anchors {
                            top: searchBar.bottom
                            topMargin: 16
                            left: parent.left
                            right: parent.right
                        }
                        height: 20
                        text: "Click on a course to select it for your schedule"
                        color: "#6b7280"
                    }

                    // courses
                    ListView {
                        id: courseListView
                        anchors {
                            top: searchHelpText.bottom
                            topMargin: 16
                            left: parent.left
                            right: parent.right
                            bottom: parent.bottom
                        }
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

                            Item {
                                id: delegateContent
                                anchors {
                                    fill: parent
                                    margins: 12
                                }

                                Rectangle {
                                    id: courseIdBox
                                    anchors {
                                        left: parent.left
                                        verticalCenter: parent.verticalCenter
                                    }
                                    width: 80
                                    height: 56
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

                                Item {
                                    id: courseInfoColumn
                                    anchors {
                                        left: courseIdBox.right
                                        leftMargin: 16
                                        right: parent.right
                                        verticalCenter: parent.verticalCenter
                                    }
                                    height: parent.height

                                    Label {
                                        id: courseTitleLabel
                                        anchors {
                                            top: parent.top
                                            topMargin: 8
                                            left: parent.left
                                            right: parent.right
                                        }
                                        height: 20
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
                                        elide: Text.ElideRight
                                    }

                                    Label {
                                        id: teacherLabel
                                        anchors {
                                            top: courseTitleLabel.bottom
                                            topMargin: 4
                                            left: parent.left
                                            right: parent.right
                                        }
                                        height: 18
                                        text: "Instructor: " + teacherName
                                        font.pixelSize: 14
                                        color: {
                                            if (selectedCoursesRepeater.count >= 7 && !courseSelectionController.isCourseSelected(originalIndex)) {
                                                return "#9ca3af"
                                            } else {
                                                return "#6b7280"
                                            }
                                        }
                                        elide: Text.ElideRight
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

                        Item {
                            anchors.centerIn: parent
                            width: parent.width * 0.8
                            height: 100
                            visible: courseListView.count === 0

                            Item {
                                id: emptyStateContent
                                anchors.centerIn: parent
                                width: 200
                                height: 100

                                Text {
                                    id: emptyIcon
                                    anchors {
                                        top: parent.top
                                        horizontalCenter: parent.horizontalCenter
                                    }
                                    height: 30
                                    text: "🔍"
                                    font.pixelSize: 24
                                }

                                Text {
                                    id: emptyTitle
                                    anchors {
                                        top: emptyIcon.bottom
                                        topMargin: 8
                                        horizontalCenter: parent.horizontalCenter
                                    }
                                    height: 25
                                    text: "No courses found"
                                    font.pixelSize: 18
                                    font.bold: true
                                    color: "#4b5563"
                                }

                                Text {
                                    id: emptySubtitle
                                    anchors {
                                        top: emptyTitle.bottom
                                        topMargin: 8
                                        horizontalCenter: parent.horizontalCenter
                                    }
                                    height: 20
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

            // Right side
            Item {
                id: rightPanel
                anchors {
                    top: parent.top
                    right: parent.right
                    bottom: parent.bottom
                    left: courseListArea.right
                    leftMargin: 16
                }

                // Selected Courses Section
                Rectangle {
                    id: selectedCoursesSection
                    anchors {
                        top: parent.top
                        left: parent.left
                        right: parent.right
                    }
                    height: (parent.height - 16) / 2
                    color: "#ffffff"
                    radius: 8
                    border.color: "#e5e7eb"

                    Item {
                        id: selectedCoursesContent
                        anchors {
                            fill: parent
                            margins: 16
                        }

                        Item {
                            id: selectedCoursesHeader
                            anchors {
                                top: parent.top
                                left: parent.left
                                right: parent.right
                            }
                            height: 30

                            Label {
                                id: selectedCoursesTitle
                                anchors {
                                    left: parent.left
                                    verticalCenter: parent.verticalCenter
                                }
                                text: "Selected Courses"
                                font.pixelSize: 18
                                font.bold: true
                                color: "#1f2937"
                            }

                            Rectangle {
                                id: courseCounter
                                anchors {
                                    right: parent.right
                                    verticalCenter: parent.verticalCenter
                                }
                                width: 60
                                height: 30
                                radius: 4
                                color: "#f3f4f6"
                                border.color: "#d1d5db"

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
                            id: selectedCoursesScrollView
                            anchors {
                                top: selectedCoursesHeader.bottom
                                topMargin: 12
                                left: parent.left
                                right: parent.right
                                bottom: parent.bottom
                            }
                            clip: true
                            ScrollBar.horizontal.policy: ScrollBar.AlwaysOff
                            contentWidth: width
                            contentHeight: selectedCoursesColumn.height

                            Item {
                                id: selectedCoursesColumn
                                width: selectedCoursesScrollView.width
                                height: selectedCoursesRepeater.count * 58

                                Repeater {
                                    id: selectedCoursesRepeater
                                    model: courseSelectionController ? courseSelectionController.selectedCoursesModel : null

                                    Rectangle {
                                        width: selectedCoursesColumn.width
                                        height: 50
                                        y: index * 58
                                        radius: 6
                                        color: "#f0f9ff"
                                        border.color: "#3b82f6"

                                        Item {
                                            id: selectedCourseContent
                                            anchors {
                                                fill: parent
                                                margins: 12
                                            }

                                            Rectangle {
                                                id: selectedCourseIdBox
                                                anchors {
                                                    left: parent.left
                                                    verticalCenter: parent.verticalCenter
                                                }
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

                                            Item {
                                                id: selectedCourseInfo
                                                anchors {
                                                    left: selectedCourseIdBox.right
                                                    leftMargin: 8
                                                    right: removeButton.left
                                                    rightMargin: 8
                                                    verticalCenter: parent.verticalCenter
                                                }
                                                height: parent.height

                                                Label {
                                                    id: selectedCourseName
                                                    anchors {
                                                        top: parent.top
                                                        topMargin: 4
                                                        left: parent.left
                                                        right: parent.right
                                                    }
                                                    height: 16
                                                    text: courseName
                                                    font.pixelSize: 14
                                                    font.bold: true
                                                    color: "#1f2937"
                                                    elide: Text.ElideRight
                                                }
                                            }

                                            Button {
                                                id: removeButton
                                                anchors {
                                                    right: parent.right
                                                    verticalCenter: parent.verticalCenter
                                                }
                                                width: 24
                                                height: 24

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
                    anchors {
                        top: selectedCoursesSection.bottom
                        topMargin: 16
                        left: parent.left
                        right: parent.right
                        bottom: parent.bottom
                    }
                    color: "#ffffff"
                    radius: 8
                    border.color: "#e5e7eb"

                    Item {
                        id: blockTimesContent
                        anchors {
                            fill: parent
                            margins: 16
                        }

                        Item {
                            id: blockTimesHeader
                            anchors {
                                top: parent.top
                                left: parent.left
                                right: parent.right
                            }
                            height: 32

                            Label {
                                id: blockTimesTitle
                                anchors {
                                    left: parent.left
                                    verticalCenter: parent.verticalCenter
                                }
                                text: "Block Times"
                                font.pixelSize: 18
                                font.bold: true
                                color: "#1f2937"
                            }

                            Button {
                                id: addBlockButton
                                anchors {
                                    right: parent.right
                                    verticalCenter: parent.verticalCenter
                                }
                                width: 120
                                height: 32

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
                            id: blockScroll
                            anchors {
                                top: blockTimesHeader.bottom
                                topMargin: 12
                                left: parent.left
                                right: parent.right
                                bottom: parent.bottom
                            }
                            clip: true
                            ScrollBar.horizontal.policy: ScrollBar.AlwaysOff
                            ScrollBar.vertical.policy: ScrollBar.AsNeeded

                            Column {
                                id: blockTimesColumn
                                width: blockScroll.width
                                spacing: 8

                                Repeater {
                                    id: blockTimesRepeater
                                    model: courseSelectionController ? courseSelectionController.blocksModel : null

                                    Rectangle {
                                        width: blockTimesColumn.width
                                        height: 60
                                        radius: 4
                                        color: "#fffbeb"
                                        border.color: "#fbbf24"
                                        border.width: 1

                                        Item {
                                            id: blockTimeContent
                                            anchors {
                                                fill: parent
                                                margins: 12
                                            }

                                            Item {
                                                id: blockTimeInfo
                                                anchors {
                                                    left: parent.left
                                                    right: removeBlockButton.left
                                                    rightMargin: 8
                                                    verticalCenter: parent.verticalCenter
                                                }
                                                height: parent.height

                                                Label {
                                                    id: blockDayLabel
                                                    anchors {
                                                        top: parent.top
                                                        topMargin: 4
                                                        left: parent.left
                                                        right: parent.right
                                                    }
                                                    height: 16
                                                    text: teacherName
                                                    font.pixelSize: 14
                                                    font.bold: true
                                                    color: "#92400e"
                                                    elide: Text.ElideRight
                                                }

                                                Label {
                                                    id: blockTimeLabel
                                                    anchors {
                                                        top: blockDayLabel.bottom
                                                        topMargin: 4
                                                        left: parent.left
                                                        right: parent.right
                                                    }
                                                    height: 14
                                                    text: courseId
                                                    font.pixelSize: 12
                                                    color: "#a16207"
                                                    elide: Text.ElideRight
                                                }
                                            }

                                            Button {
                                                id: removeBlockButton
                                                anchors {
                                                    right: parent.right
                                                    verticalCenter: parent.verticalCenter
                                                }
                                                width: 24
                                                height: 24

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

                                // Empty block times
                                Item {
                                    width: blockTimesColumn.width
                                    height: blockTimesRepeater.count === 0 ? 80 : 0
                                    visible: blockTimesRepeater.count === 0

                                    Item {
                                        id: emptyBlockContent
                                        anchors.centerIn: parent
                                        width: 200
                                        height: 80

                                        Text {
                                            id: emptyBlockIcon
                                            anchors {
                                                top: parent.top
                                                horizontalCenter: parent.horizontalCenter
                                            }
                                            height: 25
                                            text: "🚫"
                                            font.pixelSize: 20
                                        }

                                        Text {
                                            id: emptyBlockTitle
                                            anchors {
                                                top: emptyBlockIcon.bottom
                                                topMargin: 8
                                                horizontalCenter: parent.horizontalCenter
                                            }
                                            height: 18
                                            text: "No blocked times"
                                            font.pixelSize: 14
                                            color: "#6b7280"
                                            horizontalAlignment: Text.AlignHCenter
                                        }

                                        Text {
                                            id: emptyBlockSubtitle
                                            anchors {
                                                top: emptyBlockTitle.bottom
                                                topMargin: 8
                                                horizontalCenter: parent.horizontalCenter
                                            }
                                            height: 15
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
            anchors {
                bottom: parent.bottom
                left: parent.left
                right: parent.right
            }
            height: 30
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