import QtQuick 2.15
import QtQuick.Layouts 1.15
import QtQuick.Controls 2.15
import QtQuick.Controls.Basic

Popup {
    id: root

    signal saveAndClose(var filters, var blockedTimes)

    // Filter states
    property bool daysToStudyEnabled: false
    property int daysToStudyValue: 7
    property bool totalGapsEnabled: false
    property int totalGapsValue: 0
    property bool maxGapsTimeEnabled: false
    property int maxGapsTimeValue: 90
    property bool avgDayStartEnabled: false
    property int avgDayStartHour: 8
    property int avgDayStartMinute: 0
    property bool avgDayEndEnabled: false
    property int avgDayEndHour: 17
    property int avgDayEndMinute: 0

    // Window start and end time
    property int windowStartHour: 8
    property int windowStartMinute: 0
    property int windowEndHour: 9
    property int windowEndMinute: 0

    property string errorMessage: ""

    // Timer to clear error message
    Timer {
        id: errorMessageTimer
        interval: 3000 // 3 seconds
        onTriggered: {
            errorMessage = ""
        }
    }

    // Timer to scroll to bottom after adding item
    Timer {
        id: scrollToBottomTimer
        interval: 50 // Small delay to allow layout update
        onTriggered: {
            scrollView.scrollToBottom();
        }
    }

    // Window picker helpers
    function getFormattedStartTime() {
        return String(windowStartHour).padStart(2, '0') + ":" + String(windowStartMinute).padStart(2, '0');
    }

    function getFormattedEndTime() {
        return String(windowEndHour).padStart(2, '0') + ":" + String(windowEndMinute).padStart(2, '0');
    }

    function timeToMinutes(hour, minute) {
        return hour * 60 + minute;
    }

    function validateEndTime() {
        var startMinutes = timeToMinutes(windowStartHour, windowStartMinute);
        var endMinutes = timeToMinutes(windowEndHour, windowEndMinute);

        if (endMinutes <= startMinutes) {
            // Set end time to start time + 15 minutes
            var newEndMinutes = startMinutes + 15;
            windowEndHour = Math.floor(newEndMinutes / 60) % 24;
            windowEndMinute = newEndMinutes % 60;
        }
    }

    function validateStartTime() {
        var startMinutes = timeToMinutes(windowStartHour, windowStartMinute);
        var endMinutes = timeToMinutes(windowEndHour, windowEndMinute);

        if (startMinutes >= endMinutes) {
            // Set start time to end time - 15 minutes
            var newStartMinutes = endMinutes - 15;
            if (newStartMinutes < 0) {
                newStartMinutes += 24 * 60; // Handle day wraparound
            }
            windowStartHour = Math.floor(newStartMinutes / 60) % 24;
            windowStartMinute = newStartMinutes % 60;
        }
    }

    width: 400
    height: 600
    modal: true
    focus: true
    clip: true
    closePolicy: Popup.CloseOnEscape | Popup.CloseOnPressOutsideParent

    background: Rectangle {
        color: "#1f2937"
        border.color: "#d1d5db"
        border.width: 1
        radius: 6
    }

    x: (parent.width - width) / 2
    y: (parent.height - height) / 2

    property int currentPage: 0 // 0 = Block Times, 1 = Filter

    Column {
        width: parent.width
        height: parent.height
        spacing: 0

        // Header
        Rectangle {
            width: parent.width
            height: 60
            color: "transparent"

            Item {
                anchors.fill: parent
                anchors.margins: 10

                Text {
                    text: root.currentPage === 0 ? "Set Blocked Time" : "Filter Schedules"
                    font.pixelSize: 20
                    font.bold: true
                    color: "#ffffff"
                    anchors.verticalCenter: parent.verticalCenter
                    anchors.left: parent.left
                }

                Button {
                    id: saveAndClose
                    width: 40
                    height: 40
                    anchors.verticalCenter: parent.verticalCenter
                    anchors.right: parent.right

                    background: Rectangle {
                        color: "transparent"
                        radius: 4
                    }

                    // Custom content with SVG icon
                    contentItem: Item {
                        anchors.fill: parent

                        // SVG Icon
                        Image {
                            id: saveIcon
                            anchors.centerIn: parent
                            width: 24
                            height: 24
                            source: "qrc:/icons/ic-save.svg"
                            sourceSize.width: 22
                            sourceSize.height: 22
                        }

                        // Hover tooltip
                        ToolTip {
                            id: saveTooltip
                            text: "Save Preference & Close"
                            visible: saveMouseArea.containsMouse
                            delay: 500
                            timeout: 3000

                            background: Rectangle {
                                color: "#374151"
                                radius: 4
                                border.color: "#4b5563"
                            }

                            contentItem: Text {
                                text: saveTooltip.text
                                color: "white"
                                font.pixelSize: 12
                            }
                        }
                    }

                    MouseArea {
                        id: saveMouseArea
                        anchors.fill: parent
                        cursorShape: Qt.PointingHandCursor
                        hoverEnabled: true
                        onClicked: {
                            // Collect filters from the Filter page
                            var filters = {
                                daysToStudy: root.daysToStudyEnabled ? root.daysToStudyValue : null,
                                totalGaps: root.totalGapsEnabled ? root.totalGapsValue : null,
                                maxGapsTime: root.maxGapsTimeEnabled ? root.maxGapsTimeValue : null,
                                avgDayStart: root.avgDayStartEnabled ? root.avgDayStartValue : null,
                                avgDayEnd: root.avgDayEndEnabled ? root.avgDayEndValue : null
                            }

                            // Collect blocked times
                            var blockedTimes = []
                            for (var i = 0; i < blockedTimesModel.count; i++) {
                                var item = blockedTimesModel.get(i)
                                blockedTimes.push({
                                    day: item.day,
                                    startTime: item.startTime,
                                    endTime: item.endTime
                                })
                            }

                            // Emit unified signal
                            root.saveAndClose(filters, blockedTimes)
                            root.close()
                        }
                    }
                }
            }
        }

        // Navigation
        Rectangle {
            width: parent.width
            height: 50
            color: "transparent"

            Row {
                anchors.fill: parent
                spacing: 0

                // Block Times Tab
                Rectangle {
                    width: parent.width / 2
                    height: parent.height
                    color: root.currentPage === 0 ? "#374151" : "transparent"
                    border.width: root.currentPage === 0 ? 0 : 1
                    border.color: "#374151"

                    Text {
                        text: "Block Times"
                        font.pixelSize: 14
                        font.bold: root.currentPage === 0
                        color: root.currentPage === 0 ? "#ffffff" : "#9ca3af"
                        anchors.centerIn: parent
                    }

                    MouseArea {
                        anchors.fill: parent
                        cursorShape: Qt.PointingHandCursor
                        onClicked: {
                            root.currentPage = 0
                        }
                    }
                }

                // Filter Tab
                Rectangle {
                    width: parent.width / 2
                    height: parent.height
                    color: root.currentPage === 1 ? "#374151" : "transparent"
                    border.width: root.currentPage === 1 ? 0 : 1
                    border.color: "#374151"

                    Text {
                        text: "Filter"
                        font.pixelSize: 14
                        font.bold: root.currentPage === 1
                        color: root.currentPage === 1 ? "#ffffff" : "#9ca3af"
                        anchors.centerIn: parent
                    }

                    MouseArea {
                        anchors.fill: parent
                        cursorShape: Qt.PointingHandCursor
                        onClicked: {
                            root.currentPage = 1
                        }
                    }
                }
            }
        }

        // Main content
        Item {
            width: parent.width
            height: parent.height - 110 // Header + tabs + button height

            // Block Times
            Item {
                anchors.fill: parent
                visible: root.currentPage === 0

                Column {
                    anchors.fill: parent
                    anchors.margins: 10
                    spacing: 15

                    // Error message
                    Rectangle {
                        id: errorMessageContainer
                        width: parent.width
                        anchors.horizontalCenter: parent.horizontalCenter
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

                    // Blocked time slots list
                    ScrollView {
                        id: scrollView
                        width: parent.width
                        height: errorMessage === "" ? parent.height - 120 : parent.height - 175
                        anchors.horizontalCenter: parent.horizontalCenter

                        ScrollBar.horizontal.policy: ScrollBar.AlwaysOff
                        ScrollBar.vertical.policy: ScrollBar.AsNeeded

                        // scroll to bottom on new item
                        function scrollToBottom() {
                            if (contentItem && contentItem.contentHeight > height) {
                                contentItem.contentY = contentItem.contentHeight - height;
                            }
                        }

                        Column {
                            id: blockedTimesColumn
                            width: scrollView.width
                            anchors.horizontalCenter: parent.horizontalCenter
                            spacing: 10

                            Repeater {
                                model: ListModel {
                                    id: blockedTimesModel

                                    onCountChanged: {
                                        if (count > 0) {
                                            scrollToBottomTimer.start();
                                        }
                                    }
                                }

                                delegate: Item {
                                    width: blockedTimesColumn.width
                                    height: 60

                                    Rectangle {
                                        width: parent.width - 20
                                        height: 60
                                        anchors.horizontalCenter: parent.horizontalCenter
                                        color: "#374151"
                                        radius: 4

                                        RowLayout {
                                            anchors.fill: parent
                                            anchors.margins: 10

                                            Column {
                                                Layout.alignment: Qt.AlignVCenter
                                                spacing: 2

                                                Text {
                                                    text: model.day
                                                    font.pixelSize: 14
                                                    font.bold: true
                                                    color: "#ffffff"
                                                }

                                                Text {
                                                    text: model.startTime + " - " + model.endTime
                                                    font.pixelSize: 12
                                                    color: "#9ca3af"
                                                }
                                            }

                                            Item {
                                                Layout.fillWidth: true
                                            }

                                            Image {
                                                Layout.alignment: Qt.AlignVCenter
                                                width: 24
                                                height: 24
                                                source: "qrc:/icons/ic-delete.svg"
                                                sourceSize.width: 22
                                                sourceSize.height: 22

                                                MouseArea {
                                                    anchors.fill: parent
                                                    cursorShape: Qt.PointingHandCursor
                                                    onClicked: {
                                                        blockedTimesModel.remove(index)
                                                    }
                                                }
                                            }
                                        }
                                    }
                                }
                            }
                        }
                    }

                    // Add new blocked time section
                    Rectangle {
                        width: parent.width
                        height: 1
                        color: "#374151"
                        anchors.horizontalCenter: parent.horizontalCenter
                    }

                    // Input row with day selector and time pickers
                    Row {
                        anchors.horizontalCenter: parent.horizontalCenter
                        spacing: 8

                        // Day selector
                        Rectangle {
                            width: 120
                            height: 40
                            color: "#374151"
                            radius: 4
                            border.width: 1
                            border.color: "#4b5563"

                            ComboBox {
                                id: daySelector
                                anchors.fill: parent
                                model: ["Sunday", "Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday"]

                                // Style the ComboBox
                                background: Rectangle {
                                    color: "#374151"
                                    radius: 4
                                    border.width: 1
                                    border.color: "#4b5563"
                                }

                                contentItem: Text {
                                    text: daySelector.displayText
                                    font.pixelSize: 12
                                    color: "#ffffff"
                                    horizontalAlignment: Text.AlignHCenter
                                    verticalAlignment: Text.AlignVCenter
                                    leftPadding: 10
                                    rightPadding: 25
                                }

                                popup: Popup {
                                    y: daySelector.height
                                    width: daySelector.width
                                    height: contentItem.implicitHeight
                                    padding: 1

                                    background: Rectangle {
                                        color: "#374151"
                                        border.width: 1
                                        border.color: "#4b5563"
                                        radius: 4
                                    }

                                    contentItem: ListView {
                                        clip: true
                                        implicitHeight: contentHeight
                                        model: daySelector.delegateModel
                                        currentIndex: daySelector.highlightedIndex

                                        delegate: ItemDelegate {
                                            width: daySelector.width
                                            height: 30

                                            background: Rectangle {
                                                color: parent.hovered ? "#4b5563" : "#374151"
                                                radius: 2
                                            }

                                            contentItem: Text {
                                                text: modelData
                                                font.pixelSize: 12
                                                color: "#ffffff"
                                                horizontalAlignment: Text.AlignHCenter
                                                verticalAlignment: Text.AlignVCenter
                                            }
                                        }
                                    }
                                }

                                // Style the dropdown indicator
                                indicator: Text {
                                    x: daySelector.width - width - 5
                                    y: daySelector.topPadding + (daySelector.availableHeight - height) / 2
                                    text: "▼"
                                    font.pixelSize: 8
                                    color: "#ffffff"
                                }
                            }
                        }

                        // Start Time Picker
                        Rectangle {
                            width: 100
                            height: 40
                            color: "#374151"
                            radius: 4
                            border.width: 1
                            border.color: "#4b5563"

                            Row {
                                anchors.centerIn: parent
                                spacing: 2

                                // Start Hour Controls
                                Column {
                                    spacing: 2

                                    Rectangle {
                                        width: 20
                                        height: 15
                                        color: "#4b5563"
                                        radius: 2

                                        Text {
                                            text: "▲"
                                            font.pixelSize: 8
                                            color: "#ffffff"
                                            anchors.centerIn: parent
                                        }

                                        MouseArea {
                                            anchors.fill: parent
                                            onClicked: {
                                                windowStartHour = (windowStartHour + 1) % 24;
                                                validateEndTime();
                                            }
                                        }
                                    }

                                    Rectangle {
                                        width: 20
                                        height: 15
                                        color: "#4b5563"
                                        radius: 2

                                        Text {
                                            text: "▼"
                                            font.pixelSize: 8
                                            color: "#ffffff"
                                            anchors.centerIn: parent
                                        }

                                        MouseArea {
                                            anchors.fill: parent
                                            onClicked: {
                                                windowStartHour = windowStartHour > 0 ? windowStartHour - 1 : 23;
                                                validateEndTime();
                                            }
                                        }
                                    }
                                }

                                Text {
                                    text: String(windowStartHour).padStart(2, '0')
                                    font.pixelSize: 12
                                    color: "#ffffff"
                                    width: 20
                                    height: parent.height
                                    horizontalAlignment: Text.AlignHCenter
                                    verticalAlignment: Text.AlignVCenter
                                }

                                Text {
                                    text: ":"
                                    font.pixelSize: 14
                                    color: "#ffffff"
                                    anchors.verticalCenter: parent.verticalCenter
                                }

                                Text {
                                    text: String(windowStartMinute).padStart(2, '0')
                                    font.pixelSize: 12
                                    color: "#ffffff"
                                    width: 20
                                    height: parent.height
                                    horizontalAlignment: Text.AlignHCenter
                                    verticalAlignment: Text.AlignVCenter
                                }

                                // Start Minute Controls
                                Column {
                                    spacing: 2

                                    Rectangle {
                                        width: 20
                                        height: 15
                                        color: "#4b5563"
                                        radius: 2

                                        Text {
                                            text: "▲"
                                            font.pixelSize: 8
                                            color: "#ffffff"
                                            anchors.centerIn: parent
                                        }

                                        MouseArea {
                                            anchors.fill: parent
                                            onClicked: {
                                                windowStartMinute = (windowStartMinute + 15) % 60;
                                                validateEndTime();
                                            }
                                        }
                                    }

                                    Rectangle {
                                        width: 20
                                        height: 15
                                        color: "#4b5563"
                                        radius: 2

                                        Text {
                                            text: "▼"
                                            font.pixelSize: 8
                                            color: "#ffffff"
                                            anchors.centerIn: parent
                                        }

                                        MouseArea {
                                            anchors.fill: parent
                                            onClicked: {
                                                windowStartMinute = windowStartMinute >= 15 ? windowStartMinute - 15 : 45;
                                                validateEndTime();
                                            }
                                        }
                                    }
                                }
                            }
                        }

                        // Separator
                        Text {
                            text: "-"
                            font.pixelSize: 16
                            color: "#ffffff"
                            anchors.verticalCenter: parent.verticalCenter
                        }

                        // End Time Picker
                        Rectangle {
                            width: 100
                            height: 40
                            color: "#374151"
                            radius: 4
                            border.width: 1
                            border.color: "#4b5563"

                            Row {
                                anchors.centerIn: parent
                                spacing: 2

                                // End Hour Controls
                                Column {
                                    spacing: 2

                                    Rectangle {
                                        width: 20
                                        height: 15
                                        color: "#4b5563"
                                        radius: 2

                                        Text {
                                            text: "▲"
                                            font.pixelSize: 8
                                            color: "#ffffff"
                                            anchors.centerIn: parent
                                        }

                                        MouseArea {
                                            anchors.fill: parent
                                            onClicked: {
                                                windowEndHour = (windowEndHour + 1) % 24;
                                                validateStartTime();
                                            }
                                        }
                                    }

                                    Rectangle {
                                        width: 20
                                        height: 15
                                        color: "#4b5563"
                                        radius: 2

                                        Text {
                                            text: "▼"
                                            font.pixelSize: 8
                                            color: "#ffffff"
                                            anchors.centerIn: parent
                                        }

                                        MouseArea {
                                            anchors.fill: parent
                                            onClicked: {
                                                windowEndHour = windowEndHour > 0 ? windowEndHour - 1 : 23;
                                                validateStartTime();
                                            }
                                        }
                                    }
                                }

                                Text {
                                    text: String(windowEndHour).padStart(2, '0')
                                    font.pixelSize: 12
                                    color: "#ffffff"
                                    width: 20
                                    height: parent.height
                                    horizontalAlignment: Text.AlignHCenter
                                    verticalAlignment: Text.AlignVCenter
                                }

                                Text {
                                    text: ":"
                                    font.pixelSize: 14
                                    color: "#ffffff"
                                    anchors.verticalCenter: parent.verticalCenter
                                }

                                Text {
                                    text: String(windowEndMinute).padStart(2, '0')
                                    font.pixelSize: 12
                                    color: "#ffffff"
                                    width: 20
                                    height: parent.height
                                    horizontalAlignment: Text.AlignHCenter
                                    verticalAlignment: Text.AlignVCenter
                                }

                                // End Minute Controls
                                Column {
                                    spacing: 2

                                    Rectangle {
                                        width: 20
                                        height: 15
                                        color: "#4b5563"
                                        radius: 2

                                        Text {
                                            text: "▲"
                                            font.pixelSize: 8
                                            color: "#ffffff"
                                            anchors.centerIn: parent
                                        }

                                        MouseArea {
                                            anchors.fill: parent
                                            onClicked: {
                                                windowEndMinute = (windowEndMinute + 15) % 60;
                                                validateStartTime();
                                            }
                                        }
                                    }

                                    Rectangle {
                                        width: 20
                                        height: 15
                                        color: "#4b5563"
                                        radius: 2

                                        Text {
                                            text: "▼"
                                            font.pixelSize: 8
                                            color: "#ffffff"
                                            anchors.centerIn: parent
                                        }

                                        MouseArea {
                                            anchors.fill: parent
                                            onClicked: {
                                                windowEndMinute = windowEndMinute >= 15 ? windowEndMinute - 15 : 45;
                                                validateStartTime();
                                            }
                                        }
                                    }
                                }
                            }
                        }
                    }

                    // Add blocked slot button
                    Rectangle {
                        anchors.horizontalCenter: parent.horizontalCenter
                        width: parent.width
                        height: 40
                        color: "#374151"
                        radius: 4

                        Text {
                            text: "+ Add Blocked Slot"
                            font.pixelSize: 14
                            font.bold: true
                            color: "#ffffff"
                            anchors.centerIn: parent
                        }

                        MouseArea {
                            anchors.fill: parent
                            cursorShape: Qt.PointingHandCursor
                            onClicked: {
                                let exist = false;
                                for (let i = 0; i < blockedTimesModel.count; i++) {
                                    const item = blockedTimesModel.get(i);
                                    if (item.day === daySelector.currentText) {
                                        if (item.startTime === getFormattedStartTime()) {
                                            if (item.endTime === getFormattedEndTime()) {
                                                exist = true;
                                                break;
                                            }
                                        }
                                    }
                                }
                                if (!exist) {
                                    blockedTimesModel.append({
                                        "day": daySelector.currentText,
                                        "startTime": getFormattedStartTime(),
                                        "endTime": getFormattedEndTime()
                                    })
                                } else {
                                    errorMessage = "Block time already exist"
                                    errorMessageTimer.restart()
                                }
                            }
                        }
                    }
                }
            }

            // Filter Page
            Item {
                anchors.fill: parent
                visible: root.currentPage === 1

                ScrollView {
                    anchors.fill: parent
                    anchors.margins: 20
                    clip: true

                    Column {
                        width: parent.width
                        spacing: 20

                        // Days to Study filter
                        Item {
                            width: parent.width
                            height: 50

                            // Toggle Button
                            Rectangle {
                                id: daysToggle
                                width: 60
                                height: 30
                                anchors.left: parent.left
                                anchors.verticalCenter: parent.verticalCenter
                                color: daysToStudyEnabled ? "#10b981" : "#374151"
                                radius: 15
                                border.width: 2
                                border.color: daysToStudyEnabled ? "#059669" : "#4b5563"

                                Rectangle {
                                    width: 22
                                    height: 22
                                    radius: 11
                                    color: "#ffffff"
                                    x: daysToStudyEnabled ? parent.width - width - 4 : 4
                                    anchors.verticalCenter: parent.verticalCenter

                                    Behavior on x {
                                        NumberAnimation {
                                            duration: 200
                                        }
                                    }
                                }

                                MouseArea {
                                    anchors.fill: parent
                                    cursorShape: Qt.PointingHandCursor
                                    onClicked: daysToStudyEnabled = !daysToStudyEnabled
                                }
                            }

                            Text {
                                text: "Days to Study"
                                font.pixelSize: 16
                                color: "#ffffff"
                                anchors.left: daysToggle.right
                                anchors.leftMargin: 15
                                anchors.verticalCenter: parent.verticalCenter
                            }

                            // Counter
                            Rectangle {
                                width: 120
                                height: 40
                                anchors.right: parent.right
                                anchors.verticalCenter: parent.verticalCenter
                                color: "#374151"
                                radius: 4
                                border.width: 1
                                border.color: "#4b5563"

                                Row {
                                    anchors.centerIn: parent
                                    spacing: 10

                                    Rectangle {
                                        width: 30
                                        height: 30
                                        color: "#4b5563"
                                        radius: 4
                                        opacity: daysToStudyEnabled && daysToStudyValue > 1 ? 1 : 0.5

                                        Text {
                                            text: "-"
                                            font.pixelSize: 16
                                            color: "#ffffff"
                                            anchors.centerIn: parent
                                        }

                                        MouseArea {
                                            anchors.fill: parent
                                            cursorShape: Qt.PointingHandCursor
                                            enabled: daysToStudyEnabled && daysToStudyValue > 1
                                            onClicked: if (daysToStudyValue > 1) daysToStudyValue--
                                        }
                                    }

                                    Text {
                                        text: daysToStudyEnabled ? daysToStudyValue : "7"
                                        font.pixelSize: 14
                                        color: daysToStudyEnabled ? "#ffffff" : "#9ca3af"
                                        width: 30
                                        horizontalAlignment: Text.AlignHCenter
                                        anchors.verticalCenter: parent.verticalCenter
                                    }

                                    Rectangle {
                                        width: 30
                                        height: 30
                                        color: "#4b5563"
                                        radius: 4
                                        opacity: daysToStudyEnabled && daysToStudyValue < 7 ? 1 : 0.5

                                        Text {
                                            text: "+"
                                            font.pixelSize: 16
                                            color: "#ffffff"
                                            anchors.centerIn: parent
                                        }

                                        MouseArea {
                                            anchors.fill: parent
                                            cursorShape: Qt.PointingHandCursor
                                            enabled: daysToStudyEnabled && daysToStudyValue < 7
                                            onClicked: if (daysToStudyValue < 7) daysToStudyValue++
                                        }
                                    }
                                }
                            }
                        }

                        // Total Gaps filter
                        Item {
                            width: parent.width
                            height: 50

                            // Toggle Button
                            Rectangle {
                                id: gapsToggle
                                width: 60
                                height: 30
                                anchors.left: parent.left
                                anchors.verticalCenter: parent.verticalCenter
                                color: totalGapsEnabled ? "#10b981" : "#374151"
                                radius: 15
                                border.width: 2
                                border.color: totalGapsEnabled ? "#059669" : "#4b5563"

                                Rectangle {
                                    width: 22
                                    height: 22
                                    radius: 11
                                    color: "#ffffff"
                                    x: totalGapsEnabled ? parent.width - width - 4 : 4
                                    anchors.verticalCenter: parent.verticalCenter

                                    Behavior on x {
                                        NumberAnimation {
                                            duration: 200
                                        }
                                    }
                                }

                                MouseArea {
                                    anchors.fill: parent
                                    cursorShape: Qt.PointingHandCursor
                                    onClicked: totalGapsEnabled = !totalGapsEnabled
                                }
                            }

                            Text {
                                text: "Total Gaps"
                                font.pixelSize: 16
                                color: "#ffffff"
                                anchors.left: gapsToggle.right
                                anchors.leftMargin: 15
                                anchors.verticalCenter: parent.verticalCenter
                            }

                            // Counter (similar implementation as above)
                            Rectangle {
                                width: 120
                                height: 40
                                anchors.right: parent.right
                                anchors.verticalCenter: parent.verticalCenter
                                color: "#374151"
                                radius: 4
                                border.width: 1
                                border.color: "#4b5563"

                                Row {
                                    anchors.centerIn: parent
                                    spacing: 10

                                    Rectangle {
                                        width: 30
                                        height: 30
                                        color: "#4b5563"
                                        radius: 4
                                        opacity: totalGapsEnabled && totalGapsValue > 0 ? 1 : 0.5

                                        Text {
                                            text: "-"
                                            font.pixelSize: 16
                                            color: "#ffffff"
                                            anchors.centerIn: parent
                                        }

                                        MouseArea {
                                            anchors.fill: parent
                                            cursorShape: Qt.PointingHandCursor
                                            enabled: totalGapsEnabled && totalGapsValue > 0
                                            onClicked: if (totalGapsValue > 0) totalGapsValue--
                                        }
                                    }

                                    Text {
                                        text: totalGapsEnabled ? totalGapsValue : "--"
                                        font.pixelSize: 14
                                        color: totalGapsEnabled ? "#ffffff" : "#9ca3af"
                                        width: 30
                                        horizontalAlignment: Text.AlignHCenter
                                        anchors.verticalCenter: parent.verticalCenter
                                    }

                                    Rectangle {
                                        width: 30
                                        height: 30
                                        color: "#4b5563"
                                        radius: 4
                                        opacity: totalGapsEnabled ? 1 : 0.5

                                        Text {
                                            text: "+"
                                            font.pixelSize: 16
                                            color: "#ffffff"
                                            anchors.centerIn: parent
                                        }

                                        MouseArea {
                                            anchors.fill: parent
                                            cursorShape: Qt.PointingHandCursor
                                            enabled: totalGapsEnabled
                                            onClicked: totalGapsValue++
                                        }
                                    }
                                }
                            }
                        }

                        // Max Gaps Time filter
                        Item {
                            width: parent.width
                            height: 50

                            // Toggle Button
                            Rectangle {
                                id: maxGapsToggle
                                width: 60
                                height: 30
                                anchors.left: parent.left
                                anchors.verticalCenter: parent.verticalCenter
                                color: root.maxGapsTimeEnabled ? "#10b981" : "#374151"
                                radius: 15
                                border.width: 2
                                border.color: root.maxGapsTimeEnabled ? "#059669" : "#4b5563"

                                Rectangle {
                                    width: 22
                                    height: 22
                                    radius: 11
                                    color: "#ffffff"
                                    x: root.maxGapsTimeEnabled ? parent.width - width - 4 : 4
                                    anchors.verticalCenter: parent.verticalCenter

                                    Behavior on x {
                                        NumberAnimation {
                                            duration: 200
                                        }
                                    }
                                }

                                MouseArea {
                                    anchors.fill: parent
                                    cursorShape: Qt.PointingHandCursor
                                    onClicked: root.maxGapsTimeEnabled = !root.maxGapsTimeEnabled
                                }
                            }

                            Text {
                                id: gapsTimeTxt
                                text: "Max Gaps Time"
                                font.pixelSize: 16
                                color: "#ffffff"
                                anchors.left: maxGapsToggle.right
                                anchors.leftMargin: 15
                                anchors.verticalCenter: parent.verticalCenter
                            }

                            Text {
                                text: "(min)"
                                font.pixelSize: 10
                                color: "#ffffff"
                                anchors.left: gapsTimeTxt.right
                                anchors.leftMargin: 5
                                anchors.verticalCenter: parent.verticalCenter
                            }

                            // Counter
                            Rectangle {
                                width: 120
                                height: 40
                                anchors.right: parent.right
                                anchors.verticalCenter: parent.verticalCenter
                                color: "#374151"
                                radius: 4
                                border.width: 1
                                border.color: "#4b5563"

                                Row {
                                    anchors.centerIn: parent
                                    spacing: 10

                                    Rectangle {
                                        width: 30
                                        height: 30
                                        color: "#4b5563"
                                        radius: 4
                                        opacity: root.maxGapsTimeEnabled && root.maxGapsTimeValue > 90 ? 1 : 0.5

                                        Text {
                                            text: "-"
                                            font.pixelSize: 16
                                            color: "#ffffff"
                                            anchors.centerIn: parent
                                        }

                                        MouseArea {
                                            anchors.fill: parent
                                            cursorShape: Qt.PointingHandCursor
                                            enabled: root.maxGapsTimeEnabled && root.maxGapsTimeValue > 90
                                            onClicked: if (root.maxGapsTimeValue > 90) root.maxGapsTimeValue -= 5
                                        }
                                    }

                                    Text {
                                        text: root.maxGapsTimeEnabled ? root.maxGapsTimeValue : "--"
                                        font.pixelSize: 14
                                        color: root.maxGapsTimeEnabled ? "#ffffff" : "#9ca3af"
                                        width: 30
                                        horizontalAlignment: Text.AlignHCenter
                                        anchors.verticalCenter: parent.verticalCenter
                                    }

                                    Rectangle {
                                        width: 30
                                        height: 30
                                        color: "#4b5563"
                                        radius: 4
                                        opacity: root.maxGapsTimeEnabled ? 1 : 0.5

                                        Text {
                                            text: "+"
                                            font.pixelSize: 16
                                            color: "#ffffff"
                                            anchors.centerIn: parent
                                        }

                                        MouseArea {
                                            anchors.fill: parent
                                            cursorShape: Qt.PointingHandCursor
                                            enabled: root.maxGapsTimeEnabled
                                            onClicked: root.maxGapsTimeValue += 5
                                        }
                                    }
                                }
                            }
                        }

                        // Avg Day Start filter
                        Item {
                            width: parent.width
                            height: 50

                            // Toggle Button
                            Rectangle {
                                id: startToggle
                                width: 60
                                height: 30
                                anchors.left: parent.left
                                anchors.verticalCenter: parent.verticalCenter
                                color: root.avgDayStartEnabled ? "#10b981" : "#374151"
                                radius: 15
                                border.width: 2
                                border.color: root.avgDayStartEnabled ? "#059669" : "#4b5563"

                                Rectangle {
                                    width: 22
                                    height: 22
                                    radius: 11
                                    color: "#ffffff"
                                    x: root.avgDayStartEnabled ? parent.width - width - 4 : 4
                                    anchors.verticalCenter: parent.verticalCenter

                                    Behavior on x {
                                        NumberAnimation {
                                            duration: 200
                                        }
                                    }
                                }

                                MouseArea {
                                    anchors.fill: parent
                                    cursorShape: Qt.PointingHandCursor
                                    onClicked: root.avgDayStartEnabled = !root.avgDayStartEnabled
                                }
                            }

                            Text {
                                text: "Avg Day Start"
                                font.pixelSize: 16
                                color: "#ffffff"
                                anchors.left: startToggle.right
                                anchors.leftMargin: 15
                                anchors.verticalCenter: parent.verticalCenter
                            }

                            // Time Picker
                            Rectangle {
                                width: 120
                                height: 40
                                anchors.right: parent.right
                                anchors.verticalCenter: parent.verticalCenter
                                color: "#374151"
                                radius: 4
                                border.width: 1
                                border.color: "#4b5563"

                                Row {
                                    anchors.centerIn: parent
                                    spacing: 5

                                    // Hour
                                    Column {
                                        spacing: 2

                                        Rectangle {
                                            width: 20
                                            height: 15
                                            color: "#4b5563"
                                            radius: 2
                                            opacity: root.avgDayStartEnabled ? 1 : 0.5

                                            Text {
                                                text: "▲"
                                                font.pixelSize: 8
                                                color: "#ffffff"
                                                anchors.centerIn: parent
                                            }

                                            MouseArea {
                                                anchors.fill: parent
                                                enabled: root.avgDayStartEnabled
                                                onClicked: root.avgDayStartHour = (root.avgDayStartHour + 1) % 24
                                            }
                                        }

                                        Rectangle {
                                            width: 20
                                            height: 15
                                            color: "#4b5563"
                                            radius: 2
                                            opacity: root.avgDayStartEnabled ? 1 : 0.5

                                            Text {
                                                text: "▼"
                                                font.pixelSize: 8
                                                color: "#ffffff"
                                                anchors.centerIn: parent
                                            }

                                            MouseArea {
                                                anchors.fill: parent
                                                enabled: root.avgDayStartEnabled
                                                onClicked: root.avgDayStartHour = root.avgDayStartHour > 0 ? root.avgDayStartHour - 1 : 23
                                            }
                                        }
                                    }

                                    Text {
                                        text: root.avgDayStartEnabled ? String(root.avgDayStartHour).padStart(2, '0') : "--"
                                        font.pixelSize: 12
                                        color: root.avgDayStartEnabled ? "#ffffff" : "#9ca3af"
                                        width: 20
                                        height: parent.height
                                        horizontalAlignment: Text.AlignHCenter
                                        verticalAlignment: Text.AlignVCenter
                                    }

                                    Text {
                                        text: ":"
                                        font.pixelSize: 14
                                        color: root.avgDayStartEnabled ? "#ffffff" : "#9ca3af"
                                        anchors.verticalCenter: parent.verticalCenter
                                    }

                                    Text {
                                        text: root.avgDayStartEnabled ? String(root.avgDayStartMinute).padStart(2, '0') : "--"
                                        font.pixelSize: 12
                                        color: root.avgDayStartEnabled ? "#ffffff" : "#9ca3af"
                                        width: 20
                                        height: parent.height
                                        horizontalAlignment: Text.AlignHCenter
                                        verticalAlignment: Text.AlignVCenter
                                    }

                                    // Minute
                                    Column {
                                        spacing: 2

                                        Rectangle {
                                            width: 20
                                            height: 15
                                            color: "#4b5563"
                                            radius: 2
                                            opacity: root.avgDayStartEnabled ? 1 : 0.5

                                            Text {
                                                text: "▲"
                                                font.pixelSize: 8
                                                color: "#ffffff"
                                                anchors.centerIn: parent
                                            }

                                            MouseArea {
                                                anchors.fill: parent
                                                enabled: root.avgDayStartEnabled
                                                onClicked: root.avgDayStartMinute = (root.avgDayStartMinute + 15) % 60
                                            }
                                        }

                                        Rectangle {
                                            width: 20
                                            height: 15
                                            color: "#4b5563"
                                            radius: 2
                                            opacity: root.avgDayStartEnabled ? 1 : 0.5

                                            Text {
                                                text: "▼"
                                                font.pixelSize: 8
                                                color: "#ffffff"
                                                anchors.centerIn: parent
                                            }

                                            MouseArea {
                                                anchors.fill: parent
                                                enabled: root.avgDayStartEnabled
                                                onClicked: root.avgDayStartMinute = root.avgDayStartMinute >= 15 ? root.avgDayStartMinute - 15 : 45
                                            }
                                        }
                                    }
                                }
                            }
                        }

                        // Avg Day End filter
                        Item {
                            width: parent.width
                            height: 50

                            // Toggle Button
                            Rectangle {
                                id: endToggle
                                width: 60
                                height: 30
                                anchors.left: parent.left
                                anchors.verticalCenter: parent.verticalCenter
                                color: root.avgDayEndEnabled ? "#10b981" : "#374151"
                                radius: 15
                                border.width: 2
                                border.color: root.avgDayEndEnabled ? "#059669" : "#4b5563"

                                Rectangle {
                                    width: 22
                                    height: 22
                                    radius: 11
                                    color: "#ffffff"
                                    x: root.avgDayEndEnabled ? parent.width - width - 4 : 4
                                    anchors.verticalCenter: parent.verticalCenter

                                    Behavior on x {
                                        NumberAnimation {
                                            duration: 200
                                        }
                                    }
                                }

                                MouseArea {
                                    anchors.fill: parent
                                    cursorShape: Qt.PointingHandCursor
                                    onClicked: root.avgDayEndEnabled = !root.avgDayEndEnabled
                                }
                            }

                            Text {
                                text: "Avg Day End"
                                font.pixelSize: 16
                                color: "#ffffff"
                                anchors.left: endToggle.right
                                anchors.leftMargin: 15
                                anchors.verticalCenter: parent.verticalCenter
                            }

                            // Time Picker
                            Rectangle {
                                width: 120
                                height: 40
                                anchors.right: parent.right
                                anchors.verticalCenter: parent.verticalCenter
                                color: "#374151"
                                radius: 4
                                border.width: 1
                                border.color: "#4b5563"

                                Row {
                                    anchors.centerIn: parent
                                    spacing: 5

                                    // Hour
                                    Column {
                                        spacing: 2

                                        Rectangle {
                                            width: 20
                                            height: 15
                                            color: "#4b5563"
                                            radius: 2
                                            opacity: root.avgDayEndEnabled ? 1 : 0.5

                                            Text {
                                                text: "▲"
                                                font.pixelSize: 8
                                                color: "#ffffff"
                                                anchors.centerIn: parent
                                            }

                                            MouseArea {
                                                anchors.fill: parent
                                                enabled: root.avgDayEndEnabled
                                                onClicked: root.avgDayEndHour = (root.avgDayEndHour + 1) % 24
                                            }
                                        }

                                        Rectangle {
                                            width: 20
                                            height: 15
                                            color: "#4b5563"
                                            radius: 2
                                            opacity: root.avgDayEndEnabled ? 1 : 0.5

                                            Text {
                                                text: "▼"
                                                font.pixelSize: 8
                                                color: "#ffffff"
                                                anchors.centerIn: parent
                                            }

                                            MouseArea {
                                                anchors.fill: parent
                                                enabled: root.avgDayEndEnabled
                                                onClicked: root.avgDayEndHour = root.avgDayEndHour > 0 ? root.avgDayEndHour - 1 : 23
                                            }
                                        }
                                    }

                                    Text {
                                        text: root.avgDayEndEnabled ? String(root.avgDayEndHour).padStart(2, '0') : "--"
                                        font.pixelSize: 12
                                        color: root.avgDayEndEnabled ? "#ffffff" : "#9ca3af"
                                        width: 20
                                        height: parent.height
                                        horizontalAlignment: Text.AlignHCenter
                                        verticalAlignment: Text.AlignVCenter
                                    }

                                    Text {
                                        text: ":"
                                        font.pixelSize: 14
                                        color: root.avgDayEndEnabled ? "#ffffff" : "#9ca3af"
                                        anchors.verticalCenter: parent.verticalCenter
                                    }

                                    Text {
                                        text: root.avgDayEndEnabled ? String(root.avgDayEndMinute).padStart(2, '0') : "--"
                                        font.pixelSize: 12
                                        color: root.avgDayEndEnabled ? "#ffffff" : "#9ca3af"
                                        width: 20
                                        height: parent.height
                                        horizontalAlignment: Text.AlignHCenter
                                        verticalAlignment: Text.AlignVCenter
                                    }

                                    // Minute
                                    Column {
                                        spacing: 2

                                        Rectangle {
                                            width: 20
                                            height: 15
                                            color: "#4b5563"
                                            radius: 2
                                            opacity: root.avgDayEndEnabled ? 1 : 0.5

                                            Text {
                                                text: "▲"
                                                font.pixelSize: 8
                                                color: "#ffffff"
                                                anchors.centerIn: parent
                                            }

                                            MouseArea {
                                                anchors.fill: parent
                                                enabled: root.avgDayEndEnabled
                                                onClicked: root.avgDayEndMinute = (root.avgDayEndMinute + 15) % 60
                                            }
                                        }

                                        Rectangle {
                                            width: 20
                                            height: 15
                                            color: "#4b5563"
                                            radius: 2
                                            opacity: root.avgDayEndEnabled ? 1 : 0.5

                                            Text {
                                                text: "▼"
                                                font.pixelSize: 8
                                                color: "#ffffff"
                                                anchors.centerIn: parent
                                            }

                                            MouseArea {
                                                anchors.fill: parent
                                                enabled: root.avgDayEndEnabled
                                                onClicked: root.avgDayEndMinute = root.avgDayEndMinute >= 15 ? root.avgDayEndMinute - 15 : 45
                                            }
                                        }
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }
    }
}