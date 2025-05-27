import QtQuick 2.15
import QtQuick.Layouts 1.15
import QtQuick.Controls 2.15
import QtQuick.Controls.Basic

Popup {
    id: root

    signal filtersApplied(var filterData, var blockedTimes)

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

    property var originalFilterState: ({})

    // Window start and end time
    property int windowStartHour: 8
    property int windowStartMinute: 0
    property int windowEndHour: 9
    property int windowEndMinute: 0

    function saveOriginalState() {
        originalFilterState = {
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

    function restoreOriginalState() {
        root.daysToStudyEnabled = originalFilterState.daysToStudy.enabled
        root.daysToStudyValue = originalFilterState.daysToStudy.value
        root.totalGapsEnabled = originalFilterState.totalGaps.enabled
        root.totalGapsValue = originalFilterState.totalGaps.value
        root.maxGapsTimeEnabled = originalFilterState.maxGapsTime.enabled
        root.maxGapsTimeValue = originalFilterState.maxGapsTime.value
        root.avgDayStartEnabled = originalFilterState.avgDayStart.enabled
        root.avgDayStartHour = originalFilterState.avgDayStart.hour
        root.avgDayStartMinute = originalFilterState.avgDayStart.minute
        root.avgDayEndEnabled = originalFilterState.avgDayEnd.enabled
        root.avgDayEndHour = originalFilterState.avgDayEnd.hour
        root.avgDayEndMinute = originalFilterState.avgDayEnd.minute

        updateFilterMenuFromState()
    }

    function updateFilterMenuFromState() {
        filterMenu.daysToStudyEnabled = root.daysToStudyEnabled
        filterMenu.daysToStudyValue = root.daysToStudyValue
        filterMenu.totalGapsEnabled = root.totalGapsEnabled
        filterMenu.totalGapsValue = root.totalGapsValue
        filterMenu.maxGapsTimeEnabled = root.maxGapsTimeEnabled
        filterMenu.maxGapsTimeValue = root.maxGapsTimeValue
        filterMenu.avgDayStartEnabled = root.avgDayStartEnabled
        filterMenu.avgDayStartHour = root.avgDayStartHour
        filterMenu.avgDayStartMinute = root.avgDayStartMinute
        filterMenu.avgDayEndEnabled = root.avgDayEndEnabled
        filterMenu.avgDayEndHour = root.avgDayEndHour
        filterMenu.avgDayEndMinute = root.avgDayEndMinute
    }

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

    onOpened: {
        saveOriginalState()
        updateFilterMenuFromState()
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
                    id: saveAndCloseBtn
                    width: 40
                    height: 40
                    anchors.verticalCenter: parent.verticalCenter
                    anchors.right: cancleBtn.left

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
                            // Collect current filter data
                            var filterData = getCurrentFilterData()

                            // Collect blocked times from SlotBlockMenu
                            var blockedTimes = slotBlockMenu.getBlockedTimes()

                            // EMIT SIGNAL: Final save and apply filters
                            root.filtersApplied(filterData, blockedTimes)
                            root.close()
                        }
                    }
                }

                Button {
                    id: cancleBtn
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
                            id: cancleIcon
                            anchors.centerIn: parent
                            width: 24
                            height: 24
                            source: "qrc:/icons/ic-cancel.svg"
                            sourceSize.width: 22
                            sourceSize.height: 22
                        }

                        // Hover tooltip
                        ToolTip {
                            id: cancleTooltip
                            text: "Cancel Changes & Close"
                            visible: cancelMouseArea.containsMouse
                            delay: 500
                            timeout: 3000

                            background: Rectangle {
                                color: "#374151"
                                radius: 4
                                border.color: "#4b5563"
                            }

                            contentItem: Text {
                                text: cancleTooltip.text
                                color: "white"
                                font.pixelSize: 12
                            }
                        }
                    }

                    MouseArea {
                        id: cancelMouseArea
                        anchors.fill: parent
                        cursorShape: Qt.PointingHandCursor
                        hoverEnabled: true
                        onClicked: {
                            restoreOriginalState()
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
            height: parent.height - 110

            // Block Times Page
            SlotBlockMenu {
                id: slotBlockMenu
                anchors.fill: parent
                visible: root.currentPage === 0

                windowStartHour: root.windowStartHour
                windowStartMinute: root.windowStartMinute
                windowEndHour: root.windowEndHour
                windowEndMinute: root.windowEndMinute

                onWindowStartHourChanged: root.windowStartHour = windowStartHour
                onWindowStartMinuteChanged: root.windowStartMinute = windowStartMinute
                onWindowEndHourChanged: root.windowEndHour = windowEndHour
                onWindowEndMinuteChanged: root.windowEndMinute = windowEndMinute
            }

            // Filter page
            FilterMenu {
                id: filterMenu
                anchors.fill: parent
                visible: root.currentPage === 1

                // Existing filter bindings
                daysToStudyEnabled: root.daysToStudyEnabled
                daysToStudyValue: root.daysToStudyValue
                totalGapsEnabled: root.totalGapsEnabled
                totalGapsValue: root.totalGapsValue
                maxGapsTimeEnabled: root.maxGapsTimeEnabled
                maxGapsTimeValue: root.maxGapsTimeValue
                avgDayStartEnabled: root.avgDayStartEnabled
                avgDayStartHour: root.avgDayStartHour
                avgDayStartMinute: root.avgDayStartMinute
                avgDayEndEnabled: root.avgDayEndEnabled
                avgDayEndHour: root.avgDayEndHour
                avgDayEndMinute: root.avgDayEndMinute

                // Keep existing change handlers for backward compatibility
                onDaysToStudyEnabledChanged: root.daysToStudyEnabled = daysToStudyEnabled
                onDaysToStudyValueChanged: root.daysToStudyValue = daysToStudyValue
                onTotalGapsEnabledChanged: root.totalGapsEnabled = totalGapsEnabled
                onTotalGapsValueChanged: root.totalGapsValue = totalGapsValue
                onMaxGapsTimeEnabledChanged: root.maxGapsTimeEnabled = maxGapsTimeEnabled
                onMaxGapsTimeValueChanged: root.maxGapsTimeValue = maxGapsTimeValue
                onAvgDayStartEnabledChanged: root.avgDayStartEnabled = avgDayStartEnabled
                onAvgDayStartHourChanged: root.avgDayStartHour = avgDayStartHour
                onAvgDayStartMinuteChanged: root.avgDayStartMinute = avgDayStartMinute
                onAvgDayEndEnabledChanged: root.avgDayEndEnabled = avgDayEndEnabled
                onAvgDayEndHourChanged: root.avgDayEndHour = avgDayEndHour
                onAvgDayEndMinuteChanged: root.avgDayEndMinute = avgDayEndMinute
            }
        }
    }
}