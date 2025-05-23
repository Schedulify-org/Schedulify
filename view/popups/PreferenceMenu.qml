import QtQuick 2.15
import QtQuick.Controls 2.15

Popup {
    id: root

    // Expose signals for parent handling
    signal filterApplied(var filters)
    signal blockedTimesUpdated(var blockedTimes)

    width: 400
    height: 600
    modal: true
    focus: true
    clip: true
    closePolicy: Popup.CloseOnEscape | Popup.CloseOnPressOutsideParent

    // Style the popup
    background: Rectangle {
        color: "#1f2937"
        border.color: "#d1d5db"
        border.width: 1
        radius: 6
    }

    x: (parent.width - width) / 2
    y: (parent.height - height) / 2

    // Property to track current page
    property int currentPage: 0 // 0 = Block Times, 1 = Filter

    Column {
        width: parent.width
        height: parent.height
        spacing: 0

        // Header with title and close button
        Rectangle {
            width: parent.width
            height: 60
            color: "transparent"

            Row {
                anchors.fill: parent
                anchors.margins: 20

                Text {
                    text: root.currentPage === 0 ? "Set Blocked Time" : "Filter Schedules"
                    font.pixelSize: 20
                    font.bold: true
                    color: "#ffffff"
                    anchors.verticalCenter: parent.verticalCenter
                }

                Item {
                    width: parent.width - 200
                    height: 1
                }

                Text {
                    text: "✕"
                    font.pixelSize: 24
                    color: "#9ca3af"
                    anchors.verticalCenter: parent.verticalCenter
                    anchors.right: parent.right

                    MouseArea {
                        anchors.fill: parent
                        cursorShape: Qt.PointingHandCursor
                        onClicked: {
                            root.close()
                        }
                    }
                }
            }
        }

        // Tab navigation
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

        // Content area
        Item {
            width: parent.width
            height: parent.height - 110 // Header + tabs height

            // Block Times Page
            Item {
                anchors.fill: parent
                visible: root.currentPage === 0

                Column {
                    anchors.fill: parent
                    anchors.margins: 20
                    spacing: 15

                    // Blocked time slots list
                    ScrollView {
                        width: parent.width
                        height: 300
                        clip: true

                        Column {
                            width: parent.width
                            spacing: 10

                            // Example blocked time slots
                            Repeater {
                                model: ListModel {
                                    id: blockedTimesModel
                                    ListElement { day: "Monday"; startTime: "09:00"; endTime: "11:00" }
                                    ListElement { day: "Thursday"; startTime: "15:00"; endTime: "16:30" }
                                }

                                delegate: Rectangle {
                                    width: parent.width
                                    height: 60
                                    color: "#374151"
                                    radius: 4

                                    Row {
                                        anchors.fill: parent
                                        anchors.margins: 10

                                        Column {
                                            anchors.verticalCenter: parent.verticalCenter
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
                                            width: parent.width - 200
                                            height: 1
                                        }

                                        Text {
                                            text: "🗑️"
                                            font.pixelSize: 18
                                            anchors.verticalCenter: parent.verticalCenter

                                            MouseArea {
                                                anchors.fill: parent
                                                cursorShape: Qt.PointingHandCursor
                                                onClicked: {
                                                    // Delete this time slot
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
                    }

                    Row {
                        width: parent.width
                        spacing: 10

                        // Day selector
                        Rectangle {
                            width: 120
                            height: 40
                            color: "#374151"
                            radius: 4

                            ComboBox {
                                id: daySelector
                                anchors.fill: parent
                                model: ["Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday", "Sunday"]

                                contentItem: Text {
                                    text: daySelector.displayText
                                    font.pixelSize: 14
                                    color: "#ffffff"
                                    verticalAlignment: Text.AlignVCenter
                                    leftPadding: 10
                                }

                                background: Rectangle {
                                    color: "transparent"
                                }
                            }
                        }

                        // Start time
                        Rectangle {
                            width: 80
                            height: 40
                            color: "#374151"
                            radius: 4

                            TextInput {
                                id: startTimeInput
                                anchors.fill: parent
                                text: "8:00"
                                font.pixelSize: 14
                                color: "#ffffff"
                                verticalAlignment: Text.AlignVCenter
                                horizontalAlignment: Text.AlignHCenter
                            }
                        }

                        Text {
                            text: "-"
                            font.pixelSize: 16
                            color: "#ffffff"
                            anchors.verticalCenter: parent.verticalCenter
                        }

                        // End time
                        Rectangle {
                            width: 80
                            height: 40
                            color: "#374151"
                            radius: 4

                            TextInput {
                                id: endTimeInput
                                anchors.fill: parent
                                text: "9:00"
                                font.pixelSize: 14
                                color: "#ffffff"
                                verticalAlignment: Text.AlignVCenter
                                horizontalAlignment: Text.AlignHCenter
                            }
                        }
                    }

                    // Add blocked slot button
                    Rectangle {
                        width: parent.width
                        height: 50
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
                                // Add new blocked time slot
                            }
                        }
                    }

                    Item {
                        width: parent.width - 200
                        height: 1
                    }

                    // Save & Close button
                    Rectangle {
                        width: parent.width
                        height: 50
                        color: "#374151"
                        radius: 25

                        Text {
                            text: "Save & Close"
                            font.pixelSize: 16
                            font.bold: true
                            color: "#ffffff"
                            anchors.centerIn: parent
                        }

                        MouseArea {
                            anchors.fill: parent
                            cursorShape: Qt.PointingHandCursor
                            onClicked: {
                                // Save and close
                                root.close()
                            }
                        }
                    }
                }
            }

            // Filter Page
            FilterMenu {
                anchors.fill: parent
                visible: root.currentPage === 1

                onApplyFilters: {
                    root.filterApplied(filters)
                    root.close()
                }

                onCancel: {
                    root.close()
                }
            }
        }
    }
}