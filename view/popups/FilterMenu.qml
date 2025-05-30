import QtQuick 2.15
import QtQuick.Layouts 1.15
import QtQuick.Controls 2.15
import QtQuick.Controls.Basic

Popup {
    id: root

    signal filtersApplied(var filterData)

    // Filter properties
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

    function getCurrentFilterData() {
        return {
            daysToStudy: {
                enabled: root.daysToStudyEnabled,
                value: root.daysToStudyValue
            },
            totalGaps: {
                enabled: root.totalGapsEnabled,
                value: root.totalGapsValue
            },
            maxGapsTime: {
                enabled: root.maxGapsTimeEnabled,
                value: root.maxGapsTimeValue
            },
            avgDayStart: {
                enabled: root.avgDayStartEnabled,
                hour: root.avgDayStartHour,
                minute: root.avgDayStartMinute
            },
            avgDayEnd: {
                enabled: root.avgDayEndEnabled,
                hour: root.avgDayEndHour,
                minute: root.avgDayEndMinute
            }
        }
    }

    Column {
        width: parent.width
        height: parent.height
        spacing: 30

        // Header
        Rectangle {
            width: parent.width
            height: 60
            color: "transparent"

            Item {
                anchors.fill: parent
                anchors.margins: 10

                Text {
                    text: "Filter Schedules"
                    font.pixelSize: 20
                    font.bold: true
                    color: "#ffffff"
                    anchors.verticalCenter: parent.verticalCenter
                    anchors.left: parent.left
                }
            }
        }

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
                    onClicked: {
                        daysToStudyEnabled = !daysToStudyEnabled
                    }


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
                            onClicked: {
                                if (daysToStudyValue > 1) {
                                    daysToStudyValue--
                                }
                            }
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
                            onClicked: {
                                if (daysToStudyValue < 7) {
                                    daysToStudyValue++
                                }
                            }
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
                    onClicked: {
                        totalGapsEnabled = !totalGapsEnabled
                    }
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
                            onClicked: {
                                if (totalGapsValue > 0) {
                                    totalGapsValue--
                                }
                            }
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
                            onClicked: {
                                totalGapsValue++
                            }
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
                    onClicked: {
                        root.maxGapsTimeEnabled = !root.maxGapsTimeEnabled
                    }
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
                            onClicked: {
                                if (root.maxGapsTimeValue > 90) {
                                    root.maxGapsTimeValue -= 5
                                }
                            }
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
                            onClicked: {
                                root.maxGapsTimeValue += 5
                            }
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
                    onClicked: {
                        root.avgDayStartEnabled = !root.avgDayStartEnabled
                    }
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
                                onClicked: {
                                    root.avgDayStartHour = (root.avgDayStartHour + 1) % 24
                                }
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
                                onClicked: {
                                    root.avgDayStartHour = root.avgDayStartHour > 0 ? root.avgDayStartHour - 1 : 23
                                }
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
                                onClicked: {
                                    root.avgDayStartMinute = (root.avgDayStartMinute + 15) % 60
                                }
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
                                onClicked: {
                                    root.avgDayStartMinute = root.avgDayStartMinute >= 15 ? root.avgDayStartMinute - 15 : 45
                                }
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
                    onClicked: {
                        root.avgDayEndEnabled = !root.avgDayEndEnabled
                    }
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
                                onClicked: {
                                    root.avgDayEndHour = (root.avgDayEndHour + 1) % 24
                                }
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
                                onClicked: {
                                    root.avgDayEndHour = root.avgDayEndHour > 0 ? root.avgDayEndHour - 1 : 23
                                }
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
                                onClicked: {
                                    root.avgDayEndMinute = (root.avgDayEndMinute + 15) % 60
                                }
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
                                onClicked: {
                                    root.avgDayEndMinute = root.avgDayEndMinute >= 15 ? root.avgDayEndMinute - 15 : 45
                                }
                            }
                        }
                    }
                }
            }
        }

        // Apply sorting button
        Item {
            width: parent.width
            height: 60

            Rectangle {
                id: applySortingButton
                width: parent.width
                height: 50
                anchors.centerIn: parent
                anchors.bottom: parent.bottom
                color: applyMouseArea.containsMouse ? "#656363" : "#918a8a"
                radius: 6
                border.color: "#656363"
                border.width: 1

                Text {
                    text: "Apply sorting"
                    font.pixelSize: 16
                    font.bold: true
                    color: "#ffffff"
                    anchors.centerIn: parent
                }

                MouseArea {
                    id: applyMouseArea
                    anchors.fill: parent
                    hoverEnabled: true
                    cursorShape: Qt.PointingHandCursor
                    onClicked: {
                        var filterData = getCurrentFilterData()

                        root.filtersApplied(filterData)
                        root.close()
                    }
                }
            }
        }
    }
}