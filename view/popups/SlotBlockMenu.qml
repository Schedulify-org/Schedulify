import QtQuick 2.15
import QtQuick.Layouts 1.15
import QtQuick.Controls 2.15
import QtQuick.Controls.Basic

Item {
    id: root

    // Properties
    property int windowStartHour: 8
    property int windowStartMinute: 0
    property int windowEndHour: 9
    property int windowEndMinute: 0
    property string errorMessage: ""

    // Timer to scroll to bottom after adding item
    Timer {
        id: scrollToBottomTimer
        interval: 50 // Small delay to allow layout update
        onTriggered: {
            scrollView.scrollToBottom();
        }
    }

    // Helper functions
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

    function getBlockedTimes() {
        var blockedTimes = []
        for (var i = 0; i < blockedTimesModel.count; i++) {
            var item = blockedTimesModel.get(i)
            blockedTimes.push({
                day: item.day,
                startTime: item.startTime,
                endTime: item.endTime
            })
        }
        return blockedTimes
    }

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
                    }
                }
            }
        }
    }
}