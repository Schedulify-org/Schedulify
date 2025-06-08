import QtQuick 2.15
import QtQuick.Layouts 1.15
import QtQuick.Controls 2.15
import QtQuick.Controls.Basic

Popup {
    id: root

    signal sortingApplied(var sortData)

    // Primary sort properties
    property string primarySortType: ""
    property bool primarySortAscending: true

    // Secondary sort properties
    property string secondarySortType: ""
    property bool secondarySortAscending: true

    width: 450
    height: 700
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

    function getCurrentSortData() {
        return {
            primary: {
                type: root.primarySortType,
                ascending: root.primarySortAscending
            },
            secondary: {
                type: root.secondarySortType,
                ascending: root.secondarySortAscending
            }
        }
    }

    function getSortDisplayName(sortType) {
        switch(sortType) {
            case "amount_days": return "Days to Study"
            case "amount_gaps": return "Total Gaps"
            case "gaps_time": return "Max Gaps Time"
            case "avg_start": return "Avg Day Start"
            case "avg_end": return "Avg Day End"
            default: return "None"
        }
    }

    ScrollView {
        anchors.fill: parent
        anchors.margins: 20
        contentHeight: contentColumn.height

        Column {
            id: contentColumn
            width: parent.width
            spacing: 25

            // Header
            Text {
                text: "Sort Schedules"
                font.pixelSize: 22
                font.bold: true
                color: "#ffffff"
            }

            // Primary Sort Section
            Rectangle {
                width: parent.width
                height: primarySortColumn.height + 40
                color: "#374151"
                radius: 8
                border.color: "#4b5563"
                border.width: 1

                Column {
                    id: primarySortColumn
                    anchors.fill: parent
                    anchors.margins: 20
                    spacing: 15

                    Text {
                        text: "Primary Sort"
                        font.pixelSize: 18
                        font.bold: true
                        color: "#10b981"
                    }

                    // Primary Sort Type Selection
                    ComboBox {
                        id: primaryCombo
                        width: parent.width
                        height: 40
                        model: ["None", "Days to Study", "Total Gaps", "Max Gaps Time", "Avg Day Start", "Avg Day End"]

                        background: Rectangle {
                            color: "#4b5563"
                            border.color: "#6b7280"
                            border.width: 1
                            radius: 4
                        }

                        contentItem: Text {
                            text: primaryCombo.displayText
                            color: "#ffffff"
                            font.pixelSize: 14
                            verticalAlignment: Text.AlignVCenter
                            leftPadding: 10
                        }

                        onCurrentTextChanged: {
                            switch(currentText) {
                                case "Days to Study": root.primarySortType = "amount_days"; break
                                case "Total Gaps": root.primarySortType = "amount_gaps"; break
                                case "Max Gaps Time": root.primarySortType = "gaps_time"; break
                                case "Avg Day Start": root.primarySortType = "avg_start"; break
                                case "Avg Day End": root.primarySortType = "avg_end"; break
                                default: root.primarySortType = ""; break
                            }
                        }
                    }

                    // Primary Sort Direction
                    Row {
                        width: parent.width
                        spacing: 10
                        visible: root.primarySortType !== ""

                        Text {
                            text: "Direction:"
                            color: "#ffffff"
                            font.pixelSize: 14
                            anchors.verticalCenter: parent.verticalCenter
                        }

                        Rectangle {
                            width: 120
                            height: 35
                            color: "#4b5563"
                            radius: 4
                            border.width: 1
                            border.color: "#6b7280"

                            Row {
                                anchors.centerIn: parent
                                spacing: 0

                                Rectangle {
                                    width: 60
                                    height: 33
                                    color: root.primarySortAscending ? "#10b981" : "#6b7280"
                                    radius: 4

                                    Text {
                                        text: "↑"
                                        font.pixelSize: 16
                                        font.bold: true
                                        color: "#ffffff"
                                        anchors.centerIn: parent
                                    }

                                    MouseArea {
                                        anchors.fill: parent
                                        cursorShape: Qt.PointingHandCursor
                                        onClicked: root.primarySortAscending = true
                                    }
                                }

                                Rectangle {
                                    width: 60
                                    height: 33
                                    color: !root.primarySortAscending ? "#10b981" : "#6b7280"
                                    radius: 4

                                    Text {
                                        text: "↓"
                                        font.pixelSize: 16
                                        font.bold: true
                                        color: "#ffffff"
                                        anchors.centerIn: parent
                                    }

                                    MouseArea {
                                        anchors.fill: parent
                                        cursorShape: Qt.PointingHandCursor
                                        onClicked: root.primarySortAscending = false
                                    }
                                }
                            }
                        }
                    }
                }
            }

            // Secondary Sort Section
            Rectangle {
                width: parent.width
                height: secondarySortColumn.height + 40
                color: "#374151"
                radius: 8
                border.color: "#4b5563"
                border.width: 1
                opacity: root.primarySortType !== "" ? 1.0 : 0.5

                Column {
                    id: secondarySortColumn
                    anchors.fill: parent
                    anchors.margins: 20
                    spacing: 15

                    Text {
                        text: "Secondary Sort (Tie-breaker)"
                        font.pixelSize: 18
                        font.bold: true
                        color: "#f59e0b"
                    }

                    // Secondary Sort Type Selection
                    ComboBox {
                        id: secondaryCombo
                        width: parent.width
                        height: 40
                        enabled: root.primarySortType !== ""

                        model: {
                            var allOptions = ["None", "Days to Study", "Total Gaps", "Max Gaps Time", "Avg Day Start", "Avg Day End"]
                            var primaryText = getSortDisplayName(root.primarySortType)
                            return allOptions.filter(function(option) {
                                return option === "None" || option !== primaryText
                            })
                        }

                        background: Rectangle {
                            color: parent.enabled ? "#4b5563" : "#374151"
                            border.color: "#6b7280"
                            border.width: 1
                            radius: 4
                        }

                        contentItem: Text {
                            text: secondaryCombo.displayText
                            color: parent.enabled ? "#ffffff" : "#9ca3af"
                            font.pixelSize: 14
                            verticalAlignment: Text.AlignVCenter
                            leftPadding: 10
                        }

                        onCurrentTextChanged: {
                            if (!enabled) return
                            switch(currentText) {
                                case "Days to Study": root.secondarySortType = "amount_days"; break
                                case "Total Gaps": root.secondarySortType = "amount_gaps"; break
                                case "Max Gaps Time": root.secondarySortType = "gaps_time"; break
                                case "Avg Day Start": root.secondarySortType = "avg_start"; break
                                case "Avg Day End": root.secondarySortType = "avg_end"; break
                                default: root.secondarySortType = ""; break
                            }
                        }

                        // Reset secondary sort when primary changes
                        Connections {
                            target: root
                            function onPrimarySortTypeChanged() {
                                secondaryCombo.currentIndex = 0
                                root.secondarySortType = ""
                            }
                        }
                    }

                    // Secondary Sort Direction
                    Row {
                        width: parent.width
                        spacing: 10
                        visible: root.secondarySortType !== ""

                        Text {
                            text: "Direction:"
                            color: "#ffffff"
                            font.pixelSize: 14
                            anchors.verticalCenter: parent.verticalCenter
                        }

                        Rectangle {
                            width: 120
                            height: 35
                            color: "#4b5563"
                            radius: 4
                            border.width: 1
                            border.color: "#6b7280"

                            Row {
                                anchors.centerIn: parent
                                spacing: 0

                                Rectangle {
                                    width: 60
                                    height: 33
                                    color: root.secondarySortAscending ? "#10b981" : "#6b7280"
                                    radius: 4

                                    Text {
                                        text: "↑"
                                        font.pixelSize: 16
                                        font.bold: true
                                        color: "#ffffff"
                                        anchors.centerIn: parent
                                    }

                                    MouseArea {
                                        anchors.fill: parent
                                        cursorShape: Qt.PointingHandCursor
                                        onClicked: root.secondarySortAscending = true
                                    }
                                }

                                Rectangle {
                                    width: 60
                                    height: 33
                                    color: !root.secondarySortAscending ? "#10b981" : "#6b7280"
                                    radius: 4

                                    Text {
                                        text: "↓"
                                        font.pixelSize: 16
                                        font.bold: true
                                        color: "#ffffff"
                                        anchors.centerIn: parent
                                    }

                                    MouseArea {
                                        anchors.fill: parent
                                        cursorShape: Qt.PointingHandCursor
                                        onClicked: root.secondarySortAscending = false
                                    }
                                }
                            }
                        }
                    }
                }
            }

            // Sort Preview
            Rectangle {
                width: parent.width
                height: previewColumn.height + 30
                color: "#111827"
                radius: 6
                border.color: "#374151"
                border.width: 1
                visible: root.primarySortType !== ""

                Column {
                    id: previewColumn
                    anchors.fill: parent
                    anchors.margins: 15
                    spacing: 8

                    Text {
                        text: "Sort Preview:"
                        font.pixelSize: 14
                        font.bold: true
                        color: "#d1d5db"
                    }

                    Text {
                        text: "1. " + getSortDisplayName(root.primarySortType) + " (" + (root.primarySortAscending ? "Ascending" : "Descending") + ")"
                        font.pixelSize: 12
                        color: "#10b981"
                        visible: root.primarySortType !== ""
                    }

                    Text {
                        text: "2. " + getSortDisplayName(root.secondarySortType) + " (" + (root.secondarySortAscending ? "Ascending" : "Descending") + ")"
                        font.pixelSize: 12
                        color: "#f59e0b"
                        visible: root.secondarySortType !== ""
                    }
                }
            }

            // Apply and Cancel buttons
            Row {
                width: parent.width
                spacing: 15

                Rectangle {
                    width: (parent.width - 15) / 2
                    height: 50
                    color: cancelMouseArea.containsMouse ? "#dc2626" : "#ef4444"
                    radius: 6
                    border.color: "#dc2626"
                    border.width: 1

                    Text {
                        text: "Cancel"
                        font.pixelSize: 16
                        font.bold: true
                        color: "#ffffff"
                        anchors.centerIn: parent
                    }

                    MouseArea {
                        id: cancelMouseArea
                        anchors.fill: parent
                        hoverEnabled: true
                        cursorShape: Qt.PointingHandCursor
                        onClicked: root.close()
                    }
                }

                Rectangle {
                    width: (parent.width - 15) / 2
                    height: 50
                    color: applyMouseArea.containsMouse ? "#059669" : "#10b981"
                    radius: 6
                    border.color: "#059669"
                    border.width: 1
                    opacity: root.primarySortType !== "" ? 1.0 : 0.5

                    Text {
                        text: "Apply Sort"
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
                        enabled: root.primarySortType !== ""
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
}