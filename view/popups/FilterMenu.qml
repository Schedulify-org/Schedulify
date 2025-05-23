import QtQuick 2.15
import QtQuick.Controls 2.15

Item {
    id: root

    // Signals
    signal applyFilters(var filters)
    signal cancel()

    // Filter states
    property bool daysToStudyEnabled: false
    property string daysToStudyValue: "Any"

    property bool totalGapsEnabled: false
    property string totalGapsValue: "Any"

    property bool maxGapsTimeEnabled: false
    property string maxGapsTimeValue: "Any"

    property bool avgDayStartEnabled: false
    property string avgDayStartValue: "Any"

    property bool avgDayEndEnabled: false
    property string avgDayEndValue: "Any"

    Column {
        anchors.fill: parent
        anchors.margins: 20
        spacing: 20

        // Header (only shown in standalone popup mode)
        Rectangle {
            width: parent.width
            height: 40
            color: "transparent"
            visible: root.parent.objectName === "standalonePopup"

            Text {
                text: "Filter Schedules"
                font.pixelSize: 20
                font.bold: true
                color: "#ffffff"
                anchors.verticalCenter: parent.verticalCenter
            }

            Text {
                text: "✕"
                font.pixelSize: 24
                color: "#9ca3af"
                anchors.right: parent.right
                anchors.verticalCenter: parent.verticalCenter

                MouseArea {
                    anchors.fill: parent
                    cursorShape: Qt.PointingHandCursor
                    onClicked: root.cancel()
                }
            }
        }

        // Days to Study filter
        Row {
            width: parent.width
            spacing: 20

            Rectangle {
                width: 20
                height: 20
                color: "transparent"
                border.width: 2
                border.color: "#9ca3af"
                radius: 2

                CheckBox {
                    anchors.fill: parent
                    checked: root.daysToStudyEnabled
                    onClicked: {
                        root.daysToStudyEnabled = !root.daysToStudyEnabled
                    }
                }
            }

            Text {
                text: "Days to Study"
                font.pixelSize: 16
                color: "#ffffff"
                anchors.verticalCenter: parent.verticalCenter
            }

            Item {
                width: parent.width - 200
                height: 1
            }

            Rectangle {
                width: 120
                height: 40
                color: "#374151"
                radius: 4
                border.width: 1
                border.color: "#4b5563"

                ComboBox {
                    anchors.fill: parent
                    model: ["Any", "1-3", "4-6", "7+"]
                    currentIndex: 0
                    enabled: root.daysToStudyEnabled

                    onCurrentTextChanged: {
                        root.daysToStudyValue = currentText
                    }

                    contentItem: Text {
                        text: parent.displayText
                        font.pixelSize: 14
                        color: parent.enabled ? "#ffffff" : "#9ca3af"
                        verticalAlignment: Text.AlignVCenter
                        horizontalAlignment: Text.AlignHCenter
                    }

                    background: Rectangle {
                        color: "transparent"
                    }
                }
            }
        }

        // Total Gaps filter
        Row {
            width: parent.width
            spacing: 20

            Rectangle {
                width: 20
                height: 20
                color: "transparent"
                border.width: 2
                border.color: "#9ca3af"
                radius: 2

                CheckBox {
                    anchors.fill: parent
                    checked: root.totalGapsEnabled
                    onClicked: {
                        root.totalGapsEnabled = !root.totalGapsEnabled
                    }
                }
            }

            Text {
                text: "Total Gaps (Windows)"
                font.pixelSize: 16
                color: "#ffffff"
                anchors.verticalCenter: parent.verticalCenter
            }

            Item {
                width: parent.width - 200
                height: 1
            }

            Rectangle {
                width: 120
                height: 40
                color: "#374151"
                radius: 4
                border.width: 1
                border.color: "#4b5563"

                ComboBox {
                    anchors.fill: parent
                    model: ["Any", "0", "1-2", "3+"]
                    currentIndex: 0
                    enabled: root.totalGapsEnabled

                    onCurrentTextChanged: {
                        root.totalGapsValue = currentText
                    }

                    contentItem: Text {
                        text: parent.displayText
                        font.pixelSize: 14
                        color: parent.enabled ? "#ffffff" : "#9ca3af"
                        verticalAlignment: Text.AlignVCenter
                        horizontalAlignment: Text.AlignHCenter
                    }

                    background: Rectangle {
                        color: "transparent"
                    }
                }
            }
        }

        // Max Gaps Time filter
        Row {
            width: parent.width
            spacing: 20

            Rectangle {
                width: 20
                height: 20
                color: "transparent"
                border.width: 2
                border.color: "#9ca3af"
                radius: 2

                CheckBox {
                    anchors.fill: parent
                    checked: root.maxGapsTimeEnabled
                    onClicked: {
                        root.maxGapsTimeEnabled = !root.maxGapsTimeEnabled
                    }
                }
            }

            Text {
                text: "Max Gaps Time (min)"
                font.pixelSize: 16
                color: "#ffffff"
                anchors.verticalCenter: parent.verticalCenter
            }

            Item {
                width: parent.width - 200
                height: 1
            }

            Rectangle {
                width: 120
                height: 40
                color: "#374151"
                radius: 4
                border.width: 1
                border.color: "#4b5563"

                TextInput {
                    anchors.fill: parent
                    text: root.maxGapsTimeValue
                    font.pixelSize: 14
                    color: enabled ? "#ffffff" : "#9ca3af"
                    verticalAlignment: Text.AlignVCenter
                    horizontalAlignment: Text.AlignHCenter
                    enabled: root.maxGapsTimeEnabled

                    onTextChanged: {
                        root.maxGapsTimeValue = text
                    }
                }
            }
        }

        // Avg Day Start filter
        Row {
            width: parent.width
            spacing: 20

            Rectangle {
                width: 20
                height: 20
                color: "transparent"
                border.width: 2
                border.color: "#9ca3af"
                radius: 2

                CheckBox {
                    anchors.fill: parent
                    checked: root.avgDayStartEnabled
                    onClicked: {
                        root.avgDayStartEnabled = !root.avgDayStartEnabled
                    }
                }
            }

            Text {
                text: "Avg Day Start"
                font.pixelSize: 16
                color: "#ffffff"
                anchors.verticalCenter: parent.verticalCenter
            }

            Item {
                width: parent.width - 200
                height: 1
            }

            Rectangle {
                width: 120
                height: 40
                color: "#374151"
                radius: 4
                border.width: 1
                border.color: "#4b5563"

                ComboBox {
                    anchors.fill: parent
                    model: ["Any", "Before 8:00", "8:00-9:00", "After 9:00"]
                    currentIndex: 0
                    enabled: root.avgDayStartEnabled

                    onCurrentTextChanged: {
                        root.avgDayStartValue = currentText
                    }

                    contentItem: Text {
                        text: parent.displayText
                        font.pixelSize: 14
                        color: parent.enabled ? "#ffffff" : "#9ca3af"
                        verticalAlignment: Text.AlignVCenter
                        horizontalAlignment: Text.AlignHCenter
                    }

                    background: Rectangle {
                        color: "transparent"
                    }
                }
            }
        }

        // Avg Day End filter
        Row {
            width: parent.width
            spacing: 20

            Rectangle {
                width: 20
                height: 20
                color: "transparent"
                border.width: 2
                border.color: "#9ca3af"
                radius: 2

                CheckBox {
                    anchors.fill: parent
                    checked: root.avgDayEndEnabled
                    onClicked: {
                        root.avgDayEndEnabled = !root.avgDayEndEnabled
                    }
                }
            }

            Text {
                text: "Avg Day End"
                font.pixelSize: 16
                color: "#ffffff"
                anchors.verticalCenter: parent.verticalCenter
            }

            Item {
                width: parent.width - 200
                height: 1
            }

            Rectangle {
                width: 120
                height: 40
                color: "#374151"
                radius: 4
                border.width: 1
                border.color: "#4b5563"

                ComboBox {
                    anchors.fill: parent
                    model: ["Any", "Before 17:00", "17:00-19:00", "After 19:00"]
                    currentIndex: 0
                    enabled: root.avgDayEndEnabled

                    onCurrentTextChanged: {
                        root.avgDayEndValue = currentText
                    }

                    contentItem: Text {
                        text: parent.displayText
                        font.pixelSize: 14
                        color: parent.enabled ? "#ffffff" : "#9ca3af"
                        verticalAlignment: Text.AlignVCenter
                        horizontalAlignment: Text.AlignHCenter
                    }

                    background: Rectangle {
                        color: "transparent"
                    }
                }
            }
        }

        Item {
            width: parent.width - 200
            height: 1
        }

        // Action buttons
        Row {
            width: parent.width
            spacing: 10

            Rectangle {
                width: (parent.width - 10) / 2
                height: 50
                color: "transparent"
                border.width: 2
                border.color: "#9ca3af"
                radius: 25

                Text {
                    text: "Cancel"
                    font.pixelSize: 16
                    font.bold: true
                    color: "#9ca3af"
                    anchors.centerIn: parent
                }

                MouseArea {
                    anchors.fill: parent
                    cursorShape: Qt.PointingHandCursor
                    onClicked: {
                        root.cancel()
                    }
                }
            }

            Rectangle {
                width: (parent.width - 10) / 2
                height: 50
                color: "#374151"
                radius: 25

                Text {
                    text: "Apply Filters"
                    font.pixelSize: 16
                    font.bold: true
                    color: "#ffffff"
                    anchors.centerIn: parent
                }

                MouseArea {
                    anchors.fill: parent
                    cursorShape: Qt.PointingHandCursor
                    onClicked: {
                        // Collect all filter values
                        var filters = {
                            daysToStudy: root.daysToStudyEnabled ? root.daysToStudyValue : null,
                            totalGaps: root.totalGapsEnabled ? root.totalGapsValue : null,
                            maxGapsTime: root.maxGapsTimeEnabled ? root.maxGapsTimeValue : null,
                            avgDayStart: root.avgDayStartEnabled ? root.avgDayStartValue : null,
                            avgDayEnd: root.avgDayEndEnabled ? root.avgDayEndValue : null
                        }
                        root.applyFilters(filters)
                    }
                }
            }
        }
    }
}