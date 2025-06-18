import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15
import Qt.labs.qmlmodels 1.0
import QtQuick.Controls.Basic
import "popups"
import "."

Page {
    id: schedulesDisplayPage

    background: Rectangle { color: "#ffffff" }

    property var controller: schedulesDisplayController
    property var scheduleModel: controller ? controller.scheduleModel : null
    property int currentIndex: scheduleModel ? scheduleModel.currentScheduleIndex : 0
    property int totalSchedules: scheduleModel ? scheduleModel.scheduleCount : 0
    property int numDays: 7

    property int numberOfTimeSlots: 13
    property real headerHeight: 40

    property real availableTableWidth: mainContent.width - 28
    property real availableTableHeight: mainContent.height - 41

    property real timeColumnWidth: availableTableWidth * 0.12
    property real dayColumnWidth: (availableTableWidth - timeColumnWidth) / numDays
    property real uniformRowHeight: (availableTableHeight - headerHeight) / numberOfTimeSlots

    property real baseTextSize: 12

    property var logWindow: null

    // NEW: Semester navigation properties
    property string currentSemester: controller ? controller.getCurrentSemester() : "A"
    property bool allSemestersLoaded: controller ? controller.allSemestersLoaded : false

    MouseArea {
        id: outsideClickArea
        anchors.fill: parent
        z: -1
        propagateComposedEvents: true
        onPressed: function(mouse) {
            if (inputField.activeFocus) {
                var inputRect = inputField.parent
                var globalInputPos = inputRect.mapToItem(schedulesDisplayPage, 0, 0)

                if (mouse.x < globalInputPos.x ||
                    mouse.x > globalInputPos.x + inputRect.width ||
                    mouse.y < globalInputPos.y ||
                    mouse.y > globalInputPos.y + inputRect.height) {
                    inputField.focus = false
                    inputField.text = ""
                }
            }
            mouse.accepted = false
        }
    }

    Component.onDestruction: {
        if (logWindow) {
            logWindow.destroy();
            logWindow = null;
            if (logDisplayController) {
                logDisplayController.setLogWindowOpen(false);
            }
        }
    }

    Connections {
        target: scheduleModel
        function onCurrentScheduleIndexChanged() {
            if (tableModel) {
                tableModel.updateRows()
            }
        }
        function onScheduleDataChanged() {
            if (tableModel) {
                tableModel.updateRows()
            }
        }
    }

    // NEW: Connections for semester management
    Connections {
        target: controller
        function onCurrentSemesterChanged() {
            currentSemester = controller.getCurrentSemester()
            totalSchedules = scheduleModel ? scheduleModel.scheduleCount : 0
        }
        function onAllSemestersReady() {
            allSemestersLoaded = true
        }
        function onSemesterSchedulesLoaded(semester) {
            // Update button states when new semester data is loaded
        }
    }

    onDayColumnWidthChanged: {
        if (tableModel) {
            tableModel.updateRows()
        }
    }

    onUniformRowHeightChanged: {
        if (tableModel) {
            tableModel.updateRows()
        }
    }

    // Header
    Rectangle {
        id: header
        width: parent.width
        height: 120  // Increased height to accommodate semester buttons
        color: "#ffffff"
        border.color: "#e5e7eb"

        Item {
            anchors {
                left: parent.left
                right: parent.right
                top: parent.top
                bottom: parent.bottom
                margins: 15
            }

            // Top row with back button and title
            Item {
                id: topRow
                anchors {
                    left: parent.left
                    right: parent.right
                    top: parent.top
                }
                height: 40

                Button {
                    id: coursesBackButton
                    width: 40
                    height: 40
                    anchors {
                        left: parent.left
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
                        onClicked: controller.goBack()
                        cursorShape: Qt.PointingHandCursor
                    }

                    Component.onCompleted: {
                        forceActiveFocus();
                    }
                }

                Label {
                    id: titleLabel
                    text: "Generated schedules"
                    font.pixelSize: 20
                    color: "#1f2937"
                    anchors {
                        left: coursesBackButton.right
                        leftMargin: 16
                        verticalCenter: parent.verticalCenter
                    }
                }

                // Right side buttons
                RowLayout {
                    anchors {
                        right: parent.right
                        verticalCenter: parent.verticalCenter
                    }
                    spacing: 10

                    Button {
                        id: preferenceButton
                        width: 40
                        height: 40

                        background: Rectangle {
                            color: preferenceMouseArea.containsMouse ? "#a8a8a8" : "#f3f4f6"
                            radius: 10
                        }

                        contentItem: Item {
                            anchors.fill: parent

                            Image {
                                id: preferenceIcon
                                anchors.centerIn: parent
                                width: 24
                                height: 24
                                source: "qrc:/icons/ic-preference.svg"
                                sourceSize.width: 22
                                sourceSize.height: 22
                            }

                            ToolTip {
                                id: preferenceTooltip
                                text: "Set Schedule Preference"
                                visible: preferenceMouseArea.containsMouse
                                delay: 500
                                timeout: 3000

                                background: Rectangle {
                                    color: "#374151"
                                    radius: 4
                                    border.color: "#4b5563"
                                }

                                contentItem: Text {
                                    text: preferenceTooltip.text
                                    color: "white"
                                    font.pixelSize: 12
                                }
                            }
                        }

                        MouseArea {
                            id: preferenceMouseArea
                            anchors.fill: parent
                            hoverEnabled: true
                            cursorShape: Qt.PointingHandCursor
                            onClicked: sortMenu.open()
                        }
                    }

                    Button {
                        id: exportButton
                        width: 40
                        height: 40

                        property bool isExportEnabled: scheduleModel && totalSchedules > 0

                        background: Rectangle {
                            color: exportMouseArea.containsMouse ? "#a8a8a8" : "#f3f4f6"
                            radius: 10
                        }

                        contentItem: Item {
                            anchors.fill: parent

                            Image {
                                id: exportIcon
                                anchors.centerIn: parent
                                width: 24
                                height: 24
                                source: "qrc:/icons/ic-export.svg"
                                sourceSize.width: 22
                                sourceSize.height: 22
                            }

                            ToolTip {
                                id: exportTooltip
                                text: parent.parent.isExportEnabled ? "Export Schedule" : "No schedule to export"
                                visible: exportMouseArea.containsMouse
                                delay: 500
                                timeout: 3000

                                background: Rectangle {
                                    color: "#374151"
                                    radius: 4
                                    border.color: "#4b5563"
                                }

                                contentItem: Text {
                                    text: exportTooltip.text
                                    color: "white"
                                    font.pixelSize: 12
                                }
                            }
                        }

                        MouseArea {
                            id: exportMouseArea
                            anchors.fill: parent
                            hoverEnabled: true
                            cursorShape: parent.isExportEnabled ? Qt.PointingHandCursor : Qt.ForbiddenCursor
                            onClicked: {
                                if (parent.isExportEnabled) {
                                    exportMenu.currentIndex = currentIndex
                                    exportMenu.open()
                                }
                            }
                        }
                    }

                    Button {
                        id: logButtonC
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

                        MouseArea  {
                            id: logMouseArea
                            anchors.fill: parent
                            hoverEnabled: true
                            cursorShape: Qt.PointingHandCursor
                            onClicked: {
                                if (logWindow && logWindow.visible) {
                                    logWindow.raise();
                                    logWindow.requestActivate();
                                    return;
                                }

                                if (!logDisplayController.isLogWindowOpen || !logWindow) {
                                    var component = Qt.createComponent("qrc:/log_display.qml");
                                    if (component.status === Component.Ready) {
                                        logDisplayController.setLogWindowOpen(true);
                                        logWindow = component.createObject(schedulesDisplayPage);

                                        if (logWindow) {
                                            logWindow.closing.connect(function(close) {
                                                logDisplayController.setLogWindowOpen(false);
                                                logWindow = null;
                                            });

                                            logWindow.show();
                                        }
                                    } else {
                                        console.error("Error creating log window:", component.errorString());
                                    }
                                }
                            }
                        }
                    }
                }
            }

            // NEW: Semester selector row
            Item {
                id: semesterRow
                anchors {
                    left: parent.left
                    right: parent.right
                    top: topRow.bottom
                    topMargin: 15
                }
                height: 50

                Rectangle {
                    id: semesterSelectorBackground
                    anchors.centerIn: parent
                    width: semesterButtons.implicitWidth + 24
                    height: 50
                    color: "#f8fafc"
                    radius: 12
                    border.color: "#e2e8f0"
                    border.width: 1

                    RowLayout {
                        id: semesterButtons
                        anchors.centerIn: parent
                        spacing: 8

                        // Semester A Button
                        Rectangle {
                            id: semesterAButton
                            Layout.preferredWidth: 100
                            Layout.preferredHeight: 36
                            radius: 8

                            property bool isAvailable: controller ? controller.hasSchedulesForSemester("A") : false
                            property bool isActive: currentSemester === "A"
                            property bool isEnabled: isAvailable

                            color: {
                                if (!isEnabled) return "#f1f5f9"
                                if (isActive) return "#3b82f6"
                                return semesterAMouseArea.containsMouse ? "#dbeafe" : "#ffffff"
                            }

                            border.color: {
                                if (!isEnabled) return "#e2e8f0"
                                if (isActive) return "#2563eb"
                                return "#cbd5e1"
                            }
                            border.width: isActive ? 2 : 1

                            Text {
                                anchors.centerIn: parent
                                text: "Semester A"
                                font.pixelSize: 14
                                font.weight: Font.Medium
                                color: {
                                    if (!parent.isEnabled) return "#94a3b8"
                                    if (parent.isActive) return "#ffffff"
                                    return "#475569"
                                }
                            }

                            MouseArea {
                                id: semesterAMouseArea
                                anchors.fill: parent
                                hoverEnabled: parent.isEnabled
                                cursorShape: parent.isEnabled ? Qt.PointingHandCursor : Qt.ForbiddenCursor
                                enabled: parent.isEnabled
                                onClicked: {
                                    if (controller) {
                                        controller.switchToSemester("A")
                                    }
                                }
                            }

                            // Loading indicator for Semester A
                            Rectangle {
                                visible: !parent.isAvailable && currentSemester === "A"
                                anchors {
                                    right: parent.right
                                    top: parent.top
                                    margins: 4
                                }
                                width: 8
                                height: 8
                                radius: 4
                                color: "#fbbf24"

                                SequentialAnimation on opacity {
                                    running: parent.visible
                                    loops: Animation.Infinite
                                    NumberAnimation { to: 0.3; duration: 500 }
                                    NumberAnimation { to: 1.0; duration: 500 }
                                }
                            }
                        }

                        // Semester B Button
                        Rectangle {
                            id: semesterBButton
                            Layout.preferredWidth: 100
                            Layout.preferredHeight: 36
                            radius: 8

                            property bool isAvailable: controller ? controller.hasSchedulesForSemester("B") : false
                            property bool isActive: currentSemester === "B"
                            property bool isEnabled: isAvailable

                            color: {
                                if (!isEnabled) return "#f1f5f9"
                                if (isActive) return "#3b82f6"
                                return semesterBMouseArea.containsMouse ? "#dbeafe" : "#ffffff"
                            }

                            border.color: {
                                if (!isEnabled) return "#e2e8f0"
                                if (isActive) return "#2563eb"
                                return "#cbd5e1"
                            }
                            border.width: isActive ? 2 : 1

                            Text {
                                anchors.centerIn: parent
                                text: "Semester B"
                                font.pixelSize: 14
                                font.weight: Font.Medium
                                color: {
                                    if (!parent.isEnabled) return "#94a3b8"
                                    if (parent.isActive) return "#ffffff"
                                    return "#475569"
                                }
                            }

                            MouseArea {
                                id: semesterBMouseArea
                                anchors.fill: parent
                                hoverEnabled: parent.isEnabled
                                cursorShape: parent.isEnabled ? Qt.PointingHandCursor : Qt.ForbiddenCursor
                                enabled: parent.isEnabled
                                onClicked: {
                                    if (controller) {
                                        controller.switchToSemester("B")
                                    }
                                }
                            }

                            // Loading indicator for Semester B
                            Rectangle {
                                visible: !parent.isAvailable && controller && controller.hasSchedulesForSemester("A")
                                anchors {
                                    right: parent.right
                                    top: parent.top
                                    margins: 4
                                }
                                width: 8
                                height: 8
                                radius: 4
                                color: "#fbbf24"

                                SequentialAnimation on opacity {
                                    running: parent.visible
                                    loops: Animation.Infinite
                                    NumberAnimation { to: 0.3; duration: 500 }
                                    NumberAnimation { to: 1.0; duration: 500 }
                                }
                            }
                        }

                        // Summer Semester Button
                        Rectangle {
                            id: semesterSummerButton
                            Layout.preferredWidth: 120
                            Layout.preferredHeight: 36
                            radius: 8

                            property bool isAvailable: controller ? controller.hasSchedulesForSemester("SUMMER") : false
                            property bool isActive: currentSemester === "SUMMER"
                            property bool isEnabled: isAvailable

                            color: {
                                if (!isEnabled) return "#f1f5f9"
                                if (isActive) return "#3b82f6"
                                return semesterSummerMouseArea.containsMouse ? "#dbeafe" : "#ffffff"
                            }

                            border.color: {
                                if (!isEnabled) return "#e2e8f0"
                                if (isActive) return "#2563eb"
                                return "#cbd5e1"
                            }
                            border.width: isActive ? 2 : 1

                            Text {
                                anchors.centerIn: parent
                                text: "Summer"
                                font.pixelSize: 14
                                font.weight: Font.Medium
                                color: {
                                    if (!parent.isEnabled) return "#94a3b8"
                                    if (parent.isActive) return "#ffffff"
                                    return "#475569"
                                }
                            }

                            MouseArea {
                                id: semesterSummerMouseArea
                                anchors.fill: parent
                                hoverEnabled: parent.isEnabled
                                cursorShape: parent.isEnabled ? Qt.PointingHandCursor : Qt.ForbiddenCursor
                                enabled: parent.isEnabled
                                onClicked: {
                                    if (controller) {
                                        controller.switchToSemester("SUMMER")
                                    }
                                }
                            }

                            // Loading indicator for Summer
                            Rectangle {
                                visible: !parent.isAvailable && controller && (controller.hasSchedulesForSemester("A") || controller.hasSchedulesForSemester("B"))
                                anchors {
                                    right: parent.right
                                    top: parent.top
                                    margins: 4
                                }
                                width: 8
                                height: 8
                                radius: 4
                                color: "#fbbf24"

                                SequentialAnimation on opacity {
                                    running: parent.visible
                                    loops: Animation.Infinite
                                    NumberAnimation { to: 0.3; duration: 500 }
                                    NumberAnimation { to: 1.0; duration: 500 }
                                }
                            }
                        }
                    }
                }
            }

            // UPDATED: Schedule navigation (moved below semester selector)
            Rectangle {
                id: scheduleNavigation
                anchors {
                    centerIn: parent
                    bottom: parent.bottom
                }
                width: Math.max(navigationRow.implicitWidth + 32, 280)
                height: 56

                color: "#f8fafc"
                radius: 12
                border.color: "#e2e8f0"
                border.width: 1

                RowLayout {
                    id: navigationRow
                    anchors.centerIn: parent
                    spacing: 12

                    Rectangle {
                        id: prevButton
                        radius: 6
                        width: 36
                        height: 36

                        property bool isPrevEnabled: scheduleModel && totalSchedules > 0 ? scheduleModel.canGoPrevious : false

                        color: {
                            if (!isPrevEnabled) return "#e5e7eb";
                            return prevMouseArea.containsMouse ? "#35455c" : "#1f2937";
                        }

                        opacity: isPrevEnabled ? 1.0 : 0.5

                        Text {
                            text: "←"
                            anchors.centerIn: parent
                            color: parent.isPrevEnabled ? "white" : "#9ca3af"
                            font.pixelSize: 16
                            font.bold: true
                        }

                        MouseArea {
                            id: prevMouseArea
                            anchors.fill: parent
                            hoverEnabled: parent.isPrevEnabled
                            cursorShape: parent.isPrevEnabled ? Qt.PointingHandCursor : Qt.ForbiddenCursor
                            enabled: parent.isPrevEnabled
                            onClicked: {
                                if (scheduleModel) {
                                    scheduleModel.previousSchedule()
                                }
                            }
                        }
                    }

                    Label {
                        text: "Schedule"
                        font.pixelSize: 15
                        font.weight: Font.Medium
                        color: "#475569"
                    }

                    Rectangle {
                        Layout.preferredWidth: Math.max(inputField.contentWidth + 24, 60)
                        Layout.preferredHeight: 40

                        color: "#ffffff"
                        radius: 8
                        border.color: {
                            if (inputField.activeFocus) return "#3b82f6";
                            if (!inputField.isValidInput && inputField.text !== "") return "#ef4444";
                            return "#cbd5e1";
                        }
                        border.width: inputField.activeFocus ? 2 : 1

                        TextField {
                            id: inputField
                            anchors.fill: parent
                            anchors.margins: inputField.activeFocus ? 2 : 1

                            placeholderText: currentIndex + 1
                            placeholderTextColor: "#94a3b8"

                            background: Rectangle {
                                color: "transparent"
                                radius: 6
                            }

                            color: "#1f2937"
                            font.pixelSize: 14
                            font.weight: Font.Medium
                            horizontalAlignment: TextInput.AlignHCenter

                            leftPadding: 12
                            rightPadding: 12
                            topPadding: 8
                            bottomPadding: 8

                            selectByMouse: true

                            property bool isValidInput: true

                            onTextChanged: {
                                var userInput = parseInt(text)
                                isValidInput = text === "" || (!isNaN(userInput) && userInput > 0 && userInput <= totalSchedules)
                            }

                            onEditingFinished: {
                                var userInput = parseInt(inputField.text)
                                if (!isNaN(userInput) && userInput > 0 && userInput <= totalSchedules) {
                                    scheduleModel.jumpToSchedule(userInput)
                                }
                                inputField.text = ""
                                inputField.focus = false
                            }

                            Keys.onReturnPressed: {
                                var userInput = parseInt(inputField.text)
                                if (!isNaN(userInput) && userInput > 0 && userInput <= totalSchedules) {
                                    scheduleModel.jumpToSchedule(userInput)
                                }
                                inputField.text = ""
                                inputField.focus = false
                            }

                            Keys.onEscapePressed: {
                                text = ""
                                focus = false
                            }
                        }

                        ToolTip {
                            id: errorTooltip
                            text: `Please enter a number between 1 and ${totalSchedules}`
                            visible: !inputField.isValidInput && inputField.text !== "" && inputField.activeFocus
                            delay: 0
                            timeout: 0

                            background: Rectangle {
                                color: "#ef4444"
                                radius: 4
                                border.color: "#dc2626"
                            }

                            contentItem: Text {
                                text: errorTooltip.text
                                color: "white"
                                font.pixelSize: 11
                            }
                        }
                    }

                    Label {
                        text: "of"
                        font.pixelSize: 15
                        font.weight: Font.Medium
                        color: "#475569"
                    }

                    Rectangle {
                        Layout.preferredWidth: totalLabel.implicitWidth + 16
                        Layout.preferredHeight: 32

                        color: totalSchedules > 0 ? "#dbeafe" : "#f1f5f9"
                        radius: 6

                        Label {
                            id: totalLabel
                            anchors.centerIn: parent
                            text: totalSchedules > 0 ? `${totalSchedules} (${currentSemester})` : "No schedules"
                            font.pixelSize: 14
                            font.weight: Font.Medium
                            color: totalSchedules > 0 ? "#1d4ed8" : "#64748b"
                        }
                    }

                    Rectangle {
                        id: nextButton
                        radius: 6
                        width: 36
                        height: 36

                        property bool isNextEnabled: scheduleModel && totalSchedules > 0 ? scheduleModel.canGoNext : false

                        color: {
                            if (!isNextEnabled) return "#e5e7eb";
                            return nextMouseArea.containsMouse ? "#35455c" : "#1f2937";
                        }

                        opacity: isNextEnabled ? 1.0 : 0.5

                        Text {
                            text: "→"
                            anchors.centerIn: parent
                            color: parent.isNextEnabled ? "white" : "#9ca3af"
                            font.pixelSize: 16
                            font.bold: true
                        }

                        MouseArea {
                            id: nextMouseArea
                            anchors.fill: parent
                            hoverEnabled: parent.isNextEnabled
                            cursorShape: parent.isNextEnabled ? Qt.PointingHandCursor : Qt.ForbiddenCursor
                            enabled: parent.isNextEnabled
                            onClicked: {
                                if (scheduleModel) {
                                    scheduleModel.nextSchedule()
                                }
                            }
                        }
                    }
                }
            }
        }
    }

    // export menu link
    ExportMenu {
        id: exportMenu
        parent: Overlay.overlay

        onPrintRequested: {
            if (controller) {
                controller.printScheduleDirectly()
            }
        }

        onSaveAsPngRequested: {
            if (schedulesDisplayController && tableContent) {
                schedulesDisplayController.captureAndSave(tableContent)
            }
        }

        onSaveAsCsvRequested: {
            if (controller) {
                controller.saveScheduleAsCSV()
            }
        }
    }

    // Sorting menu link
    SortMenu {
        id: sortMenu
        parent: Overlay.overlay
        onSortingApplied: function(sortingData) {
            if (controller) {
                controller.applySorting(sortingData)
            }
        }
    }

    // Main content
    Rectangle{
        id: mainContent
        anchors {
            top: header.bottom
            left: parent.left
            right: parent.right
            bottom: footer.top
        }

        // Trim text based on stages (full, mid, min)
        function getFormattedText(courseName, courseId, building, room, type, cellWidth, cellHeight) {
            if (type === "Block") {
                return "<b style='font-size:" + baseTextSize + "px'>" + courseName + "</b>";
            }

            var charWidth = baseTextSize * 0.6;
            var maxCharsPerLine = Math.floor((cellWidth - 12) / charWidth); // Account for padding

            var line1, line2;

            // Full stage
            var fullLine1 = courseName + " (" + courseId + ")";
            var fullLine2 = "Building: " + building + ", Room: " + room;

            if (fullLine1.length <= maxCharsPerLine && fullLine2.length <= maxCharsPerLine) {
                line1 = fullLine1;
                line2 = fullLine2;
            } else {
                // Mid stage
                var courseNameLimit = Math.max(3, maxCharsPerLine - courseId.length - 6); // Minimum 3 chars for "..."
                var trimmedName = courseName.length > courseNameLimit ?
                    courseName.substring(0, courseNameLimit - 3) + "..." :
                    courseName;
                var midLine1 = trimmedName + " (" + courseId + ")";
                var midLine2 = "B: " + building + ", R: " + room;

                if (midLine1.length <= maxCharsPerLine && midLine2.length <= maxCharsPerLine) {
                    line1 = midLine1;
                    line2 = midLine2;
                } else {
                    // Min stage
                    line1 = courseId;

                    // original min building & room
                    var minLine2 = building + ", " + room;

                    if (minLine2.length <= maxCharsPerLine) {
                        line2 = minLine2;
                    } else {
                        // Trim building if needed
                        var roomSpace = room.length + 2;
                        var buildingLimit = maxCharsPerLine - roomSpace;

                        if (buildingLimit > 3) {
                            var trimmedBuilding = building.length > buildingLimit ?
                                building.substring(0, buildingLimit - 3) + "..." :
                                building;
                            line2 = trimmedBuilding + ", " + room;
                        } else {
                            if (building.length <= maxCharsPerLine) {
                                line2 = building;
                            } else {
                                line2 = building.substring(0, maxCharsPerLine - 3) + "...";
                            }
                        }
                    }
                }
            }

            return "<b style='font-size:" + baseTextSize + "px'>" + line1 + "</b><br>" +
                "<span style='font-size:" + baseTextSize + "px'>" + line2 + "</span>";
        }

        Item {
            anchors.fill: parent
            anchors.margins: 10

            Rectangle {
                id: noSchedulesMessage
                anchors.fill: parent
                visible: totalSchedules <= 0
                color: "#f9fafb"
                border.color: "#e5e7eb"
                border.width: 2
                radius: 8

                Text {
                    anchors.centerIn: parent
                    text: controller && controller.hasSchedulesForSemester(currentSemester)
                        ? "No schedules available for " + currentSemester
                        : "Schedules for " + currentSemester + " are being generated..."
                    font.pixelSize: 24
                    font.bold: true
                    color: "#6b7280"
                    horizontalAlignment: Text.AlignHCenter
                    verticalAlignment: Text.AlignVCenter
                }

                // Loading animation when schedules are being generated
                Rectangle {
                    visible: !controller || !controller.hasSchedulesForSemester(currentSemester)
                    anchors {
                        horizontalCenter: parent.horizontalCenter
                        top: parent.verticalCenter
                        topMargin: 40
                    }
                    width: 32
                    height: 32
                    radius: 16
                    color: "#3b82f6"

                    SequentialAnimation on opacity {
                        running: parent.visible
                        loops: Animation.Infinite
                        NumberAnimation { to: 0.3; duration: 800 }
                        NumberAnimation { to: 1.0; duration: 800 }
                    }

                    RotationAnimator on rotation {
                        running: parent.visible
                        from: 0
                        to: 360
                        duration: 2000
                        loops: Animation.Infinite
                    }
                }
            }

            Column {
                id: tableContent
                anchors.fill: parent
                visible: totalSchedules > 0
                spacing: 1

                // Header row
                Row {
                    id: dayHeaderRow
                    height: headerHeight
                    spacing: 1
                    width: parent.width

                    Rectangle {
                        width: timeColumnWidth
                        height: headerHeight
                        color: "#e5e7eb"
                        border.color: "#d1d5db"
                        radius: 4

                        Text {
                            anchors.centerIn: parent
                            text: "Hour/Day"
                            font.pixelSize: baseTextSize
                            font.bold: true
                            color: "#4b5563"
                        }
                    }

                    Repeater {
                        model: ["Sunday", "Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday"]

                        Rectangle {
                            width: dayColumnWidth
                            height: headerHeight
                            color: "#e5e7eb"
                            border.color: "#d1d5db"
                            radius: 4

                            Text {
                                anchors.centerIn: parent
                                text: modelData
                                font.pixelSize: baseTextSize
                                font.bold: true
                                color: "#4b5563"
                                wrapMode: Text.WordWrap
                                elide: Text.ElideRight
                            }
                        }
                    }
                }

                // Schedule table
                TableView {
                    id: scheduleTable
                    width: parent.width
                    height: parent.height - headerHeight - 1
                    clip: true
                    rowSpacing: 1
                    columnSpacing: 1
                    interactive: false

                    property var timeSlots: [
                        "8:00-9:00", "9:00-10:00", "10:00-11:00", "11:00-12:00",
                        "12:00-13:00", "13:00-14:00", "14:00-15:00", "15:00-16:00",
                        "16:00-17:00", "17:00-18:00", "18:00-19:00", "19:00-20:00",
                        "20:00-21:00"
                    ]

                    columnWidthProvider: function(col) {
                        return col === 0 ? timeColumnWidth : dayColumnWidth;
                    }

                    rowHeightProvider: function(row) {
                        return uniformRowHeight;
                    }

                    model: TableModel {
                        id: tableModel
                        TableModelColumn { display: "timeSlot" }
                        TableModelColumn { display: "sunday" }
                        TableModelColumn { display: "monday" }
                        TableModelColumn { display: "tuesday" }
                        TableModelColumn { display: "wednesday" }
                        TableModelColumn { display: "thursday" }
                        TableModelColumn { display: "friday" }
                        TableModelColumn { display: "saturday" }

                        function updateRows() {
                            let rows = [];
                            const timeSlots = scheduleTable.timeSlots;
                            const days = ["sunday", "monday", "tuesday", "wednesday", "thursday", "friday", "saturday"];

                            for (let i = 0; i < timeSlots.length; i++) {
                                let row = { timeSlot: timeSlots[i] };
                                for (let day of days) {
                                    row[day] = "";
                                    row[day + "_type"] = "";
                                    row[day + "_courseName"] = "";
                                    row[day + "_courseId"] = "";
                                    row[day + "_building"] = "";
                                    row[day + "_room"] = "";
                                }
                                rows.push(row);
                            }

                            if (totalSchedules > 0 && scheduleModel) {
                                for (let day = 0; day < 7; day++) {
                                    let dayName = days[day];
                                    let items = scheduleModel.getCurrentDayItems(day);

                                    for (let itemIndex = 0; itemIndex < items.length; itemIndex++) {
                                        let item = items[itemIndex];
                                        let start = parseInt(item.start.split(":")[0]);
                                        let end = parseInt(item.end.split(":")[0]);

                                        for (let hour = start; hour < end; hour++) {
                                            for (let rowIndex = 0; rowIndex < timeSlots.length; rowIndex++) {
                                                let slot = timeSlots[rowIndex];
                                                let slotStart = parseInt(slot.split("-")[0].split(":")[0]);
                                                let slotEnd = parseInt(slot.split("-")[1].split(":")[0]);

                                                if (hour >= slotStart && hour < slotEnd) {
                                                    if (!rows[rowIndex][dayName + "_type"]) {
                                                        rows[rowIndex][dayName + "_type"] = item.type;
                                                        rows[rowIndex][dayName + "_courseName"] = item.courseName;
                                                        rows[rowIndex][dayName + "_courseId"] = item.raw_id;
                                                        rows[rowIndex][dayName + "_building"] = item.building;
                                                        rows[rowIndex][dayName + "_room"] = item.room;

                                                        rows[rowIndex][dayName] = mainContent.getFormattedText(
                                                            item.courseName,
                                                            item.raw_id,
                                                            item.building,
                                                            item.room,
                                                            item.type,
                                                            dayColumnWidth,
                                                            uniformRowHeight
                                                        );
                                                    }
                                                }
                                            }
                                        }
                                    }
                                }
                            }
                            tableModel.rows = rows;
                        }

                        Component.onCompleted: updateRows()
                    }

                    delegate: Rectangle {
                        implicitHeight: uniformRowHeight
                        implicitWidth: model.column === 0 ? timeColumnWidth : dayColumnWidth
                        border.width: 1
                        border.color: "#e0e0e0"
                        radius: 4

                        property string columnName: {
                            switch(model.column) {
                                case 1: return "sunday_type";
                                case 2: return "monday_type";
                                case 3: return "tuesday_type";
                                case 4: return "wednesday_type";
                                case 5: return "thursday_type";
                                case 6: return "friday_type";
                                case 7: return "saturday_type";
                                default: return "";
                            }
                        }

                        property string itemType: {
                            if (columnName && model.row !== undefined) {
                                let rowData = parent.parent.model.rows[model.row];
                                if (rowData && rowData[columnName]) {
                                    return rowData[columnName];
                                }
                            }
                            return "";
                        }

                        // Updated color assignment in the delegate Rectangle
                        color: {
                            if (model.column === 0) {
                                return "#d1d5db";
                            }

                            if (!model.display || String(model.display).trim().length === 0) {
                                return "#ffffff";
                            }

                            switch(itemType) {
                                case "Lecture": return "#b0e8ff";           // Light blue
                                case "Lab": return "#abffc6";               // Light green
                                case "Tutorial": return "#edc8ff";          // Light purple
                                case "Block": return "#7a7a7a";             // Gray
                                case "Departmental Session": return "#ffd6a5";  // Light orange
                                case "Reinforcement": return "#ffaaa5";     // Light red
                                case "Guidance": return "#a5d6ff";          // Light sky blue
                                case "Optional Colloquium": return "#d4a5ff"; // Light lavender
                                case "Registration": return "#ffa5d4";      // Light pink
                                case "Thesis": return "#a5ffd4";            // Light mint
                                case "Project": return "#d4ffa5";           // Light lime
                                default: return "#e5e7eb";                  // Light gray for unknown types
                            }
                        }

                        ToolTip {
                            id: sessionTooltip
                            text: itemType || "No session type"
                            visible: sessionMouseArea.containsMouse && itemType !== ""
                            delay: 500
                            timeout: 3000

                            background: Rectangle {
                                color: "#374151"
                                radius: 4
                                border.color: "#4b5563"
                            }

                            contentItem: Text {
                                text: sessionTooltip.text
                                color: "white"
                                font.pixelSize: 12
                            }
                        }

                        Text {
                            anchors.fill: parent
                            wrapMode: Text.WordWrap
                            verticalAlignment: Text.AlignVCenter
                            horizontalAlignment: Text.AlignHCenter
                            padding: 4
                            font.pixelSize: baseTextSize
                            textFormat: Text.RichText
                            text: model.display ? String(model.display) : ""
                            color: itemType === "Block" ? "#ffffff": "#000000"
                            clip: true
                        }

                        MouseArea {
                            id: sessionMouseArea
                            anchors.fill: parent
                            hoverEnabled: true
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