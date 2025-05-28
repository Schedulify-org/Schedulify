import QtQuick 2.15
import QtQuick.Layouts 1.15
import QtQuick.Controls 2.15
import QtQuick.Controls.Basic

Popup {
    id: root

    signal blockTimeAdded(string day, string startTime, string endTime)

    // Properties
    property int startHour: 8
    property int endHour: 9
    property string errorMessage: ""

    width: 400
    height: 350
    modal: true
    focus: true
    clip: true
    closePolicy: Popup.CloseOnEscape | Popup.CloseOnPressOutsideParent

    background: Rectangle {
        color: "#ffffff"
        border.color: "#d1d5db"
        border.width: 1
        radius: 8

        // Drop shadow effect
        Rectangle {
            anchors.fill: parent
            anchors.margins: -2
            color: "transparent"
            border.color: "#00000020"
            border.width: 1
            radius: 10
            z: -1
        }
    }

    x: (parent.width - width) / 2
    y: (parent.height - height) / 2

    // Timer to clear error message
    Timer {
        id: errorMessageTimer
        interval: 3000
        onTriggered: {
            errorMessage = ""
        }
    }

    // Helper functions
    function getFormattedTime(hour) {
        return String(hour).padStart(2, '0') + ":00";
    }

    function validateTimes() {
        if (endHour <= startHour) {
            endHour = startHour + 1;
            if (endHour > 23) {
                endHour = 23;
                startHour = 22;
            }
        }
    }

    function addBlockTime() {
        // Validate inputs
        if (daySelector.currentIndex < 0) {
            errorMessage = "Please select a day";
            errorMessageTimer.restart();
            return;
        }

        validateTimes();

        var day = daySelector.currentText;
        var startTime = getFormattedTime(startHour);
        var endTime = getFormattedTime(endHour);

        // Emit signal with the block time data
        root.blockTimeAdded(day, startTime, endTime);

        // Reset form
        daySelector.currentIndex = 0;
        startHour = 8;
        endHour = 9;
        errorMessage = "";

        // Close popup
        root.close();
    }

    Column {
        anchors.fill: parent
        anchors.margins: 20
        spacing: 20

        // Header
        Row {
            width: parent.width
            spacing: 10

            Text {
                text: "Add Block Time"
                font.pixelSize: 20
                font.bold: true
                color: "#1f2937"
                anchors.verticalCenter: parent.verticalCenter
            }

            Item {
                width: parent.width - 200
            }

            Button {
                id: closeButton
                width: 30
                height: 30
                anchors.verticalCenter: parent.verticalCenter

                background: Rectangle {
                    color: closeMouseArea.containsMouse ? "#f3f4f6" : "transparent"
                    radius: 15
                }

                contentItem: Text {
                    text: "×"
                    font.pixelSize: 20
                    color: "#6b7280"
                    horizontalAlignment: Text.AlignHCenter
                    verticalAlignment: Text.AlignVCenter
                }

                MouseArea {
                    id: closeMouseArea
                    anchors.fill: parent
                    hoverEnabled: true
                    onClicked: root.close()
                    cursorShape: Qt.PointingHandCursor
                }
            }
        }

        // Error message
        Rectangle {
            id: errorMessageContainer
            width: parent.width
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

        // Day selection
        Column {
            width: parent.width
            spacing: 8

            Label {
                text: "Day"
                font.pixelSize: 14
                font.bold: true
                color: "#374151"
            }

            Rectangle {
                width: parent.width
                height: 45
                color: "#f9fafb"
                radius: 6
                border.width: 1
                border.color: "#d1d5db"

                ComboBox {
                    id: daySelector
                    anchors.fill: parent
                    model: ["Sunday", "Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday"]

                    background: Rectangle {
                        color: "transparent"
                        radius: 6
                    }

                    contentItem: Text {
                        text: daySelector.displayText
                        font.pixelSize: 14
                        color: "#1f2937"
                        horizontalAlignment: Text.AlignLeft
                        verticalAlignment: Text.AlignVCenter
                        leftPadding: 12
                        rightPadding: 30
                    }

                    popup: Popup {
                        y: daySelector.height
                        width: daySelector.width
                        height: contentItem.implicitHeight
                        padding: 1

                        background: Rectangle {
                            color: "#ffffff"
                            border.width: 1
                            border.color: "#d1d5db"
                            radius: 6
                        }

                        contentItem: ListView {
                            clip: true
                            implicitHeight: contentHeight
                            model: daySelector.delegateModel
                            currentIndex: daySelector.highlightedIndex

                            delegate: ItemDelegate {
                                width: daySelector.width
                                height: 40

                                background: Rectangle {
                                    color: parent.hovered ? "#f3f4f6" : "#ffffff"
                                    radius: 4
                                }

                                contentItem: Text {
                                    text: modelData
                                    font.pixelSize: 14
                                    color: "#1f2937"
                                    horizontalAlignment: Text.AlignLeft
                                    verticalAlignment: Text.AlignVCenter
                                    leftPadding: 12
                                }
                            }
                        }
                    }

                    indicator: Text {
                        x: daySelector.width - width - 10
                        y: daySelector.topPadding + (daySelector.availableHeight - height) / 2
                        text: "▼"
                        font.pixelSize: 10
                        color: "#6b7280"
                    }
                }
            }
        }

        // Time selection
        Row {
            width: parent.width
            spacing: 20

            // Start time
            Column {
                width: (parent.width - 20) / 2
                spacing: 8

                Label {
                    text: "Start Time"
                    font.pixelSize: 14
                    font.bold: true
                    color: "#374151"
                }

                Rectangle {
                    width: parent.width
                    height: 45
                    color: "#f9fafb"
                    radius: 6
                    border.width: 1
                    border.color: "#d1d5db"

                    Row {
                        anchors.centerIn: parent
                        spacing: 5

                        // Hour display
                        Text {
                            text: String(startHour).padStart(2, '0') + ":00"
                            font.pixelSize: 16
                            color: "#1f2937"
                            width: 50
                            horizontalAlignment: Text.AlignHCenter
                            anchors.verticalCenter: parent.verticalCenter
                        }

                        // Hour controls
                        Column {
                            spacing: 2

                            Rectangle {
                                width: 25
                                height: 18
                                color: upHourMouseArea.containsMouse ? "#e5e7eb" : "#f3f4f6"
                                radius: 3
                                border.width: 1
                                border.color: "#d1d5db"

                                Text {
                                    text: "▲"
                                    font.pixelSize: 10
                                    color: "#374151"
                                    anchors.centerIn: parent
                                }

                                MouseArea {
                                    id: upHourMouseArea
                                    anchors.fill: parent
                                    hoverEnabled: true
                                    onClicked: {
                                        if (startHour < 23) {
                                            startHour = startHour + 1;
                                            validateTimes();
                                        }
                                    }
                                    cursorShape: Qt.PointingHandCursor
                                }
                            }

                            Rectangle {
                                width: 25
                                height: 18
                                color: downHourMouseArea.containsMouse ? "#e5e7eb" : "#f3f4f6"
                                radius: 3
                                border.width: 1
                                border.color: "#d1d5db"

                                Text {
                                    text: "▼"
                                    font.pixelSize: 10
                                    color: "#374151"
                                    anchors.centerIn: parent
                                }

                                MouseArea {
                                    id: downHourMouseArea
                                    anchors.fill: parent
                                    hoverEnabled: true
                                    onClicked: {
                                        if (startHour > 0) {
                                            startHour = startHour - 1;
                                            validateTimes();
                                        }
                                    }
                                    cursorShape: Qt.PointingHandCursor
                                }
                            }
                        }
                    }
                }
            }

            // End time
            Column {
                width: (parent.width - 20) / 2
                spacing: 8

                Label {
                    text: "End Time"
                    font.pixelSize: 14
                    font.bold: true
                    color: "#374151"
                }

                Rectangle {
                    width: parent.width
                    height: 45
                    color: "#f9fafb"
                    radius: 6
                    border.width: 1
                    border.color: "#d1d5db"

                    Row {
                        anchors.centerIn: parent
                        spacing: 5

                        // Hour display
                        Text {
                            text: String(endHour).padStart(2, '0') + ":00"
                            font.pixelSize: 16
                            color: "#1f2937"
                            width: 50
                            horizontalAlignment: Text.AlignHCenter
                            anchors.verticalCenter: parent.verticalCenter
                        }

                        // Hour controls
                        Column {
                            spacing: 2

                            Rectangle {
                                width: 25
                                height: 18
                                color: upEndHourMouseArea.containsMouse ? "#e5e7eb" : "#f3f4f6"
                                radius: 3
                                border.width: 1
                                border.color: "#d1d5db"

                                Text {
                                    text: "▲"
                                    font.pixelSize: 10
                                    color: "#374151"
                                    anchors.centerIn: parent
                                }

                                MouseArea {
                                    id: upEndHourMouseArea
                                    anchors.fill: parent
                                    hoverEnabled: true
                                    onClicked: {
                                        if (endHour < 23) {
                                            endHour = endHour + 1;
                                        }
                                    }
                                    cursorShape: Qt.PointingHandCursor
                                }
                            }

                            Rectangle {
                                width: 25
                                height: 18
                                color: downEndHourMouseArea.containsMouse ? "#e5e7eb" : "#f3f4f6"
                                radius: 3
                                border.width: 1
                                border.color: "#d1d5db"

                                Text {
                                    text: "▼"
                                    font.pixelSize: 10
                                    color: "#374151"
                                    anchors.centerIn: parent
                                }

                                MouseArea {
                                    id: downEndHourMouseArea
                                    anchors.fill: parent
                                    hoverEnabled: true
                                    onClicked: {
                                        if (endHour > 1) {
                                            endHour = endHour - 1;
                                            validateTimes();
                                        }
                                    }
                                    cursorShape: Qt.PointingHandCursor
                                }
                            }
                        }
                    }
                }
            }
        }

        // Action buttons
        Row {
            width: parent.width
            spacing: 12
            anchors.horizontalCenter: parent.horizontalCenter

            Button {
                id: cancelButton
                width: (parent.width - 12) / 2
                height: 45

                background: Rectangle {
                    color: cancelMouseArea.containsMouse ? "#f3f4f6" : "#ffffff"
                    radius: 6
                    border.width: 1
                    border.color: "#d1d5db"
                }

                contentItem: Text {
                    text: "Cancel"
                    font.pixelSize: 14
                    color: "#374151"
                    horizontalAlignment: Text.AlignHCenter
                    verticalAlignment: Text.AlignVCenter
                }

                MouseArea {
                    id: cancelMouseArea
                    anchors.fill: parent
                    hoverEnabled: true
                    onClicked: root.close()
                    cursorShape: Qt.PointingHandCursor
                }
            }

            Button {
                id: addButton
                width: (parent.width - 12) / 2
                height: 45

                background: Rectangle {
                    color: addMouseArea.containsMouse ? "#1d4ed8" : "#2563eb"
                    radius: 6
                }

                contentItem: Text {
                    text: "Add Block Time"
                    font.pixelSize: 14
                    font.bold: true
                    color: "#ffffff"
                    horizontalAlignment: Text.AlignHCenter
                    verticalAlignment: Text.AlignVCenter
                }

                MouseArea {
                    id: addMouseArea
                    anchors.fill: parent
                    hoverEnabled: true
                    onClicked: addBlockTime()
                    cursorShape: Qt.PointingHandCursor
                }
            }
        }
    }
}