import QtQuick 2.15
import QtQuick.Layouts 1.15
import QtQuick.Controls 2.15
import QtQuick.Controls.Basic

Popup {
    id: root

    signal sortingApplied(var sortData)

    // Sort properties
    property bool daysToStudyEnabled: false
    property bool daysToStudyAscending: true
    property bool totalGapsEnabled: false
    property bool totalGapsAscending: true
    property bool maxGapsTimeEnabled: false
    property bool maxGapsTimeAscending: true
    property bool avgDayStartEnabled: false
    property bool avgDayStartAscending: true
    property bool avgDayEndEnabled: false
    property bool avgDayEndAscending: true

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

    // make the selection unique
    function disableAllSortsExcept(keepEnabled) {
        if (keepEnabled !== "daysToStudy") daysToStudyEnabled = false
        if (keepEnabled !== "totalGaps") totalGapsEnabled = false
        if (keepEnabled !== "maxGapsTime") maxGapsTimeEnabled = false
        if (keepEnabled !== "avgDayStart") avgDayStartEnabled = false
        if (keepEnabled !== "avgDayEnd") avgDayEndEnabled = false
    }

    function getCurrentSortData() {
        return {
            amount_days: {
                enabled: root.daysToStudyEnabled,
                ascending: root.daysToStudyAscending
            },
            amount_gaps: {
                enabled: root.totalGapsEnabled,
                ascending: root.totalGapsAscending
            },
            gaps_time: {
                enabled: root.maxGapsTimeEnabled,
                ascending: root.maxGapsTimeAscending
            },
            avg_start: {
                enabled: root.avgDayStartEnabled,
                ascending: root.avgDayStartAscending
            },
            avg_end: {
                enabled: root.avgDayEndEnabled,
                ascending: root.avgDayEndAscending
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
                    text: "Sort Schedules"
                    font.pixelSize: 20
                    font.bold: true
                    color: "#ffffff"
                    anchors.verticalCenter: parent.verticalCenter
                    anchors.left: parent.left
                }
            }
        }

        // Days to Study sort
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
                        if (!daysToStudyEnabled) {
                            disableAllSortsExcept("daysToStudy")
                            daysToStudyEnabled = true
                        } else {
                            daysToStudyEnabled = false
                        }
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

            // Ascending/Descending Toggle
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
                    spacing: 0

                    Rectangle {
                        width: 60
                        height: 38
                        color: daysToStudyEnabled && daysToStudyAscending ? "#10b981" : "#4b5563"
                        radius: 4
                        border.width: 1
                        border.color: "#374151"

                        Text {
                            text: "↑"
                            font.pixelSize: 18
                            font.bold: true
                            color: "#ffffff"
                            anchors.centerIn: parent
                        }

                        MouseArea {
                            anchors.fill: parent
                            cursorShape: Qt.PointingHandCursor
                            enabled: daysToStudyEnabled
                            onClicked: {
                                daysToStudyAscending = true
                            }
                        }
                    }

                    Rectangle {
                        width: 60
                        height: 38
                        color: daysToStudyEnabled && !daysToStudyAscending ? "#10b981" : "#4b5563"
                        radius: 4
                        border.width: 1
                        border.color: "#374151"

                        Text {
                            text: "↓"
                            font.pixelSize: 18
                            font.bold: true
                            color: "#ffffff"
                            anchors.centerIn: parent
                        }

                        MouseArea {
                            anchors.fill: parent
                            cursorShape: Qt.PointingHandCursor
                            enabled: daysToStudyEnabled
                            onClicked: {
                                daysToStudyAscending = false
                            }
                        }
                    }
                }
            }
        }

        // Total Gaps sort
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
                        if (!totalGapsEnabled) {
                            disableAllSortsExcept("totalGaps")
                            totalGapsEnabled = true
                        } else {
                            totalGapsEnabled = false
                        }
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

            // Ascending/Descending Toggle
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
                    spacing: 0

                    Rectangle {
                        width: 60
                        height: 38
                        color: totalGapsEnabled && totalGapsAscending ? "#10b981" : "#4b5563"
                        radius: 4
                        border.width: 1
                        border.color: "#374151"

                        Text {
                            text: "↑"
                            font.pixelSize: 18
                            font.bold: true
                            color: "#ffffff"
                            anchors.centerIn: parent
                        }

                        MouseArea {
                            anchors.fill: parent
                            cursorShape: Qt.PointingHandCursor
                            enabled: totalGapsEnabled
                            onClicked: {
                                totalGapsAscending = true
                            }
                        }
                    }

                    Rectangle {
                        width: 60
                        height: 38
                        color: totalGapsEnabled && !totalGapsAscending ? "#10b981" : "#4b5563"
                        radius: 4
                        border.width: 1
                        border.color: "#374151"

                        Text {
                            text: "↓"
                            font.pixelSize: 18
                            font.bold: true
                            color: "#ffffff"
                            anchors.centerIn: parent
                        }

                        MouseArea {
                            anchors.fill: parent
                            cursorShape: Qt.PointingHandCursor
                            enabled: totalGapsEnabled
                            onClicked: {
                                totalGapsAscending = false
                            }
                        }
                    }
                }
            }
        }

        // Max Gaps Time sort
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
                        if (!root.maxGapsTimeEnabled) {
                            disableAllSortsExcept("maxGapsTime")
                            root.maxGapsTimeEnabled = true
                        } else {
                            root.maxGapsTimeEnabled = false
                        }
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

            // Ascending/Descending Toggle
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
                    spacing: 0

                    Rectangle {
                        width: 60
                        height: 38
                        color: root.maxGapsTimeEnabled && root.maxGapsTimeAscending ? "#10b981" : "#4b5563"
                        radius: 4
                        border.width: 1
                        border.color: "#374151"

                        Text {
                            text: "↑"
                            font.pixelSize: 18
                            font.bold: true
                            color: "#ffffff"
                            anchors.centerIn: parent
                        }


                        MouseArea {
                            anchors.fill: parent
                            cursorShape: Qt.PointingHandCursor
                            enabled: root.maxGapsTimeEnabled
                            onClicked: {
                                root.maxGapsTimeAscending = true
                            }
                        }
                    }

                    Rectangle {
                        width: 60
                        height: 38
                        color: root.maxGapsTimeEnabled && !root.maxGapsTimeAscending ? "#10b981" : "#4b5563"
                        radius: 4
                        border.width: 1
                        border.color: "#374151"

                        Text {
                            text: "↓"
                            font.pixelSize: 18
                            font.bold: true
                            color: "#ffffff"
                            anchors.centerIn: parent
                        }

                        MouseArea {
                            anchors.fill: parent
                            cursorShape: Qt.PointingHandCursor
                            enabled: root.maxGapsTimeEnabled
                            onClicked: {
                                root.maxGapsTimeAscending = false
                            }
                        }
                    }
                }
            }
        }

        // Avg Day Start sort
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
                        if (!root.avgDayStartEnabled) {
                            disableAllSortsExcept("avgDayStart")
                            root.avgDayStartEnabled = true
                        } else {
                            root.avgDayStartEnabled = false
                        }
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

            // Ascending/Descending Toggle
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
                    spacing: 0

                    Rectangle {
                        width: 60
                        height: 38
                        color: root.avgDayStartEnabled && root.avgDayStartAscending ? "#10b981" : "#4b5563"
                        radius: 4
                        border.width: 1
                        border.color: "#374151"

                        Text {
                            text: "↑"
                            font.pixelSize: 18
                            font.bold: true
                            color: "#ffffff"
                            anchors.centerIn: parent
                        }

                        MouseArea {
                            anchors.fill: parent
                            cursorShape: Qt.PointingHandCursor
                            enabled: root.avgDayStartEnabled
                            onClicked: {
                                root.avgDayStartAscending = true
                            }
                        }
                    }

                    Rectangle {
                        width: 60
                        height: 38
                        color: root.avgDayStartEnabled && !root.avgDayStartAscending ? "#10b981" : "#4b5563"
                        radius: 4
                        border.width: 1
                        border.color: "#374151"

                        Text {
                            text: "↓"
                            font.pixelSize: 18
                            font.bold: true
                            color: "#ffffff"
                            anchors.centerIn: parent
                        }

                        MouseArea {
                            anchors.fill: parent
                            cursorShape: Qt.PointingHandCursor
                            enabled: root.avgDayStartEnabled
                            onClicked: {
                                root.avgDayStartAscending = false
                            }
                        }
                    }
                }
            }
        }

        // Avg Day End sort
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
                        if (!root.avgDayEndEnabled) {
                            disableAllSortsExcept("avgDayEnd")
                            root.avgDayEndEnabled = true
                        } else {
                            root.avgDayEndEnabled = false
                        }
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

            // Ascending/Descending Toggle
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
                    spacing: 0

                    Rectangle {
                        width: 60
                        height: 38
                        color: root.avgDayEndEnabled && root.avgDayEndAscending ? "#10b981" : "#4b5563"
                        radius: 4
                        border.width: 1
                        border.color: "#374151"

                        Text {
                            text: "↑"
                            font.pixelSize: 18
                            font.bold: true
                            color: "#ffffff"
                            anchors.centerIn: parent
                        }

                        MouseArea {
                            anchors.fill: parent
                            cursorShape: Qt.PointingHandCursor
                            enabled: root.avgDayEndEnabled
                            onClicked: {
                                root.avgDayEndAscending = true
                            }
                        }
                    }

                    Rectangle {
                        width: 60
                        height: 38
                        color: root.avgDayEndEnabled && !root.avgDayEndAscending ? "#10b981" : "#4b5563"
                        radius: 4
                        border.width: 1
                        border.color: "#374151"

                        Text {
                            text: "↓"
                            font.pixelSize: 18
                            font.bold: true
                            color: "#ffffff"
                            anchors.centerIn: parent
                        }

                        MouseArea {
                            anchors.fill: parent
                            cursorShape: Qt.PointingHandCursor
                            enabled: root.avgDayEndEnabled
                            onClicked: {
                                root.avgDayEndAscending = false
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
                        var sortData = getCurrentSortData()

                        root.sortingApplied(sortData)
                        root.close()
                    }
                }
            }
        }
    }
}