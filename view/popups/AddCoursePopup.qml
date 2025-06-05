import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15
import QtQuick.Controls.Basic

Popup {
    id: addCoursePopup
    width: Math.min(parent.width * 0.95, 1000)
    height: Math.min(parent.height * 0.95, 850)
    anchors.centerIn: parent
    modal: true
    focus: true
    clip: true
    closePolicy: Popup.CloseOnEscape | Popup.CloseOnPressOutside

    property alias courseName: courseNameField.text
    property alias courseId: courseIdField.text
    property alias teacherName: teacherField.text

    // Error handling properties
    property string errorMessage: ""

    signal courseCreated(string courseName, string courseId, string teacherName, var sessionGroups)

    // Clear all data
    function resetPopup() {
        courseNameField.text = ""
        courseIdField.text = ""
        teacherField.text = ""
        errorMessage = ""
        groupsModel.clear()
        groupsModel.append({
            "groupType": "Lecture",
            "groupIndex": 0
        })
    }

    // Show error message
    function showError(message) {
        errorMessage = message
        errorTimer.restart()
    }

    // Timer for error messages
    Timer {
        id: errorTimer
        interval: 4000
        onTriggered: {
            errorMessage = ""
        }
    }

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

    background: Rectangle {
        color: "#ffffff"
        border.color: "#e5e7eb"
        border.width: 2
        radius: 16

        Rectangle {
            anchors.fill: parent
            anchors.margins: -4
            color: "transparent"
            border.color: "#00000015"
            border.width: 2
            radius: 20
            z: -1
        }

        Rectangle {
            anchors.fill: parent
            anchors.margins: -8
            color: "transparent"
            border.color: "#00000010"
            border.width: 2
            radius: 24
            z: -2
        }
    }

    // Main content section
    Column {
        anchors.fill: parent
        anchors.margins: 24
        anchors.bottomMargin: 90
        spacing: 20

        // Header
        Rectangle {
            width: parent.width
            height: 60
            color: "transparent"

            Text {
                anchors {
                    left: parent.left
                    verticalCenter: parent.verticalCenter
                }
                text: "Add New Course"
                font.pixelSize: 28
                font.bold: true
                color: "#1f2937"
            }

            Button {
                anchors {
                    right: parent.right
                    verticalCenter: parent.verticalCenter
                }
                width: 40
                height: 40

                background: Rectangle {
                    color: closeMouseArea.containsMouse ? "#f3f4f6" : "transparent"
                    radius: 20
                    border.width: 1
                    border.color: closeMouseArea.containsMouse ? "#d1d5db" : "transparent"
                }

                contentItem: Text {
                    text: "×"
                    font.pixelSize: 24
                    color: "#6b7280"
                    horizontalAlignment: Text.AlignHCenter
                    verticalAlignment: Text.AlignVCenter
                }

                MouseArea {
                    id: closeMouseArea
                    anchors.fill: parent
                    hoverEnabled: true
                    onClicked: {
                        resetPopup()
                        addCoursePopup.close()
                    }
                    cursorShape: Qt.PointingHandCursor
                }
            }
        }

        // Error message
        Rectangle {
            width: parent.width
            height: errorMessage === "" ? 0 : 50
            visible: errorMessage !== ""
            color: "#fef2f2"
            radius: 8
            border.color: "#fecaca"
            border.width: 1

            Text {
                anchors.centerIn: parent
                text: errorMessage
                color: "#dc2626"
                font.pixelSize: 14
                font.bold: true
            }
        }

        // Course Information Section
        Rectangle {
            width: parent.width
            height: 230
            color: "#f8fafc"
            radius: 12
            border.width: 1
            border.color: "#e2e8f0"

            Column {
                anchors.fill: parent
                anchors.margins: 20
                spacing: 20

                Text {
                    text: "Course Information"
                    font.pixelSize: 18
                    font.bold: true
                    color: "#1e293b"
                }

                GridLayout {
                    width: parent.width
                    columns: 2
                    columnSpacing: 20
                    rowSpacing: 16

                    // Course ID
                    Column {
                        Layout.preferredWidth: 150
                        spacing: 8

                        Text {
                            text: "Course ID *"
                            font.pixelSize: 14
                            font.bold: true
                            color: "#374151"
                        }

                        Rectangle {
                            width: parent.width
                            height: 44
                            color: "#ffffff"
                            radius: 8
                            border.width: 2
                            border.color: courseIdField.focus ? "#3b82f6" : "#e5e7eb"

                            TextField {
                                id: courseIdField
                                anchors.fill: parent
                                placeholderText: "e.g., 12345"
                                placeholderTextColor: "#9CA3AF"
                                validator: RegularExpressionValidator { regularExpression: /[0-9]{0,5}/ }
                                background: Rectangle { color: "transparent" }
                                color: "#1f2937"
                                font.pixelSize: 14
                                leftPadding: 16
                                rightPadding: 16
                                topPadding: 12
                                bottomPadding: 12
                            }
                        }
                    }

                    // Course Name
                    Column {
                        Layout.fillWidth: true
                        spacing: 8

                        Text {
                            text: "Course Name *"
                            font.pixelSize: 14
                            font.bold: true
                            color: "#374151"
                        }

                        Rectangle {
                            width: parent.width
                            height: 44
                            color: "#ffffff"
                            radius: 8
                            border.width: 2
                            border.color: courseNameField.focus ? "#3b82f6" : "#e5e7eb"

                            TextField {
                                id: courseNameField
                                anchors.fill: parent
                                placeholderText: "e.g., Introduction to Computer Science"
                                placeholderTextColor: "#9CA3AF"
                                background: Rectangle { color: "transparent" }
                                color: "#1f2937"
                                font.pixelSize: 14
                                leftPadding: 16
                                rightPadding: 16
                                topPadding: 12
                                bottomPadding: 12
                            }
                        }
                    }

                    // Teacher
                    Column {
                        Layout.fillWidth: true
                        Layout.columnSpan: 2
                        spacing: 8

                        Text {
                            text: "Teacher *"
                            font.pixelSize: 14
                            font.bold: true
                            color: "#374151"
                        }

                        Rectangle {
                            width: parent.width
                            height: 44
                            color: "#ffffff"
                            radius: 8
                            border.width: 2
                            border.color: teacherField.focus ? "#3b82f6" : "#e5e7eb"

                            TextField {
                                id: teacherField
                                anchors.fill: parent
                                placeholderText: "e.g., Prof. prof"
                                placeholderTextColor: "#9CA3AF"
                                background: Rectangle { color: "transparent" }
                                color: "#1f2937"
                                font.pixelSize: 14
                                leftPadding: 16
                                rightPadding: 16
                                topPadding: 12
                                bottomPadding: 12
                            }
                        }
                    }
                }
            }
        }

        // Session Groups Section Header
        Rectangle {
            width: parent.width
            height: 20
            color: "transparent"

            Text {
                anchors {
                    left: parent.left
                    verticalCenter: parent.verticalCenter
                }
                text: "Session Groups"
                font.pixelSize: 22
                font.bold: true
                color: "#1e293b"
            }

            Button {
                anchors {
                    right: parent.right
                    verticalCenter: parent.verticalCenter
                }
                width: 140
                height: 40

                background: Rectangle {
                    color: addGroupMouseArea.containsMouse ? "#3b82f6" : "#4f46e5"
                    radius: 8
                    border.width: 0

                    // Subtle gradient effect
                    Rectangle {
                        anchors.fill: parent
                        color: "transparent"
                        radius: 8
                        border.width: 1
                        border.color: "#ffffff20"
                    }
                }

                contentItem: RowLayout {
                    spacing: 8
                    anchors.centerIn: parent

                    Text {
                        text: "+"
                        font.pixelSize: 18
                        font.bold: true
                        color: "#ffffff"
                    }

                    Text {
                        text: "Add Group"
                        font.pixelSize: 14
                        font.bold: true
                        color: "#ffffff"
                    }
                }

                MouseArea {
                    id: addGroupMouseArea
                    anchors.fill: parent
                    hoverEnabled: true
                    onClicked: {
                        groupsModel.append({
                            "groupType": "Lecture",
                            "groupIndex": groupsModel.count
                        })
                    }
                    cursorShape: Qt.PointingHandCursor
                }
            }
        }

        // Scrollable Groups List Section
        ScrollView {
            id: groupsScrollView
            width: parent.width
            height: parent.height - 350
            clip: true
            contentWidth: availableWidth

            Column {
                width: groupsScrollView.width
                spacing: 20

                Repeater {
                    id: groupsRepeater
                    model: ListModel {
                        id: groupsModel
                        ListElement {
                            groupType: "Lecture"
                            groupIndex: 0
                        }
                    }

                    delegate: Rectangle {
                        width: parent.width
                        color: "#ffffff"
                        border.color: "#e2e8f0"
                        border.width: 2
                        radius: 16
                        height: groupContent.implicitHeight + 32

                        // Subtle shadow
                        Rectangle {
                            anchors.fill: parent
                            anchors.margins: -2
                            color: "transparent"
                            border.color: "#00000008"
                            border.width: 1
                            radius: 18
                            z: -1
                        }

                        property alias groupType: groupTypeCombo.currentText
                        property var sessionsList: []

                        Column {
                            id: groupContent
                            anchors.fill: parent
                            anchors.margins: 16
                            spacing: 16

                            // Group Header
                            Rectangle {
                                width: parent.width
                                height: 50
                                color: "#f1f5f9"
                                radius: 12
                                border.width: 1
                                border.color: "#e2e8f0"

                                RowLayout {
                                    anchors.fill: parent
                                    anchors.margins: 12
                                    spacing: 16

                                    ComboBox {
                                        id: groupTypeCombo
                                        Layout.preferredWidth: 120
                                        model: ["Lecture", "Tutorial", "Lab"]
                                        currentIndex: {
                                            switch (groupType) {
                                                case "Tutorial":
                                                    return 1
                                                case "Lab":
                                                    return 2
                                                default:
                                                    return 0
                                            }
                                        }

                                        background: Rectangle {
                                            color: "#ffffff"
                                            border.color: "#d1d5db"
                                            border.width: 2
                                            radius: 8
                                        }

                                        contentItem: Text {
                                            text: parent.currentText
                                            font.pixelSize: 14
                                            font.bold: true
                                            color: "#1e293b"
                                            leftPadding: 12
                                            verticalAlignment: Text.AlignVCenter
                                        }

                                        onCurrentTextChanged: {
                                            groupsModel.setProperty(index, "groupType", currentText)
                                        }
                                    }

                                    Text {
                                        text: "Group " + (groupIndex + 1)
                                        font.pixelSize: 16
                                        font.bold: true
                                        color: "#64748b"
                                    }

                                    Item {
                                        Layout.fillWidth: true
                                    }

                                    Button {
                                        Layout.preferredWidth: 36
                                        Layout.preferredHeight: 36
                                        enabled: groupsModel.count > 1

                                        background: Rectangle {
                                            color: parent.enabled ? (deleteGroupMouseArea.containsMouse ? "#fee2e2" : "#f8fafc") : "#f3f4f6"
                                            radius: 18
                                            border.width: 1
                                            border.color: parent.enabled ? "#e5e7eb" : "#d1d5db"
                                        }

                                        contentItem: Text {
                                            text: "×"
                                            font.pixelSize: 18
                                            font.bold: true
                                            color: parent.enabled ? "#ef4444" : "#d1d5db"
                                            horizontalAlignment: Text.AlignHCenter
                                            verticalAlignment: Text.AlignVCenter
                                        }

                                        MouseArea {
                                            id: deleteGroupMouseArea
                                            anchors.fill: parent
                                            hoverEnabled: true
                                            onClicked: {
                                                if (groupsModel && groupsModel.count > 1) {
                                                    groupsModel.remove(index)
                                                    for (var i = 0; i < groupsModel.count; i++) {
                                                        groupsModel.setProperty(i, "groupIndex", i)
                                                    }
                                                } else {
                                                    showError("A course must have at least one group")
                                                }
                                            }
                                            cursorShape: Qt.PointingHandCursor
                                        }
                                    }
                                }
                            }

                            // Sessions for this group
                            Column {
                                width: parent.width
                                spacing: 12

                                Repeater {
                                    id: sessionsRepeater
                                    model: ListModel {
                                        id: sessionsModel
                                        Component.onCompleted: {
                                            append({
                                                "day": "Sunday",
                                                "startHour": 9,
                                                "endHour": 10,
                                                "building": "",
                                                "room": ""
                                            })
                                        }
                                    }

                                    delegate: Rectangle {
                                        width: parent.width
                                        height: 60
                                        color: "#fafbfc"
                                        border.color: "#e2e8f0"
                                        border.width: 1
                                        radius: 12

                                        function validateTimes() {
                                            if (endHour <= startHour) {
                                                endHour = startHour + 1;
                                                if (endHour > 23) {
                                                    endHour = 23;
                                                    startHour = 22;
                                                }
                                            }
                                            sessionsModel.setProperty(index, "startHour", startHour)
                                            sessionsModel.setProperty(index, "endHour", endHour)
                                        }

                                        RowLayout {
                                            anchors.fill: parent
                                            anchors.margins: 12
                                            spacing: 12

                                            // Day Selection
                                            ComboBox {
                                                id: dayCombo
                                                Layout.preferredWidth: 110
                                                Layout.preferredHeight: 36
                                                model: ["Sunday", "Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday"]
                                                currentIndex: {
                                                    var days = ["Sunday", "Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday"]
                                                    return Math.max(0, days.indexOf(day))
                                                }

                                                background: Rectangle {
                                                    color: "#ffffff"
                                                    border.color: "#d1d5db"
                                                    border.width: 1
                                                    radius: 8
                                                }

                                                contentItem: Text {
                                                    text: parent.currentText
                                                    font.pixelSize: 13
                                                    color: "#374151"
                                                    leftPadding: 8
                                                    rightPadding: 20
                                                    verticalAlignment: Text.AlignVCenter
                                                    elide: Text.ElideRight
                                                }

                                                onCurrentTextChanged: {
                                                    sessionsModel.setProperty(index, "day", currentText)
                                                }
                                            }

                                            // Start Time
                                            Rectangle {
                                                Layout.preferredWidth: 80
                                                Layout.preferredHeight: 36
                                                color: "#ffffff"
                                                radius: 8
                                                border.width: 1
                                                border.color: "#d1d5db"

                                                RowLayout {
                                                    anchors.fill: parent
                                                    anchors.margins: 4
                                                    spacing: 2

                                                    Text {
                                                        Layout.fillWidth: true
                                                        text: String(startHour).padStart(2, '0') + ":00"
                                                        font.pixelSize: 13
                                                        color: "#1f2937"
                                                        horizontalAlignment: Text.AlignHCenter
                                                        verticalAlignment: Text.AlignVCenter
                                                    }

                                                    Column {
                                                        Layout.preferredWidth: 16
                                                        spacing: 1

                                                        Rectangle {
                                                            width: 16
                                                            height: 12
                                                            color: upStartHourMouseArea.containsMouse ? "#e5e7eb" : "#f3f4f6"
                                                            radius: 4
                                                            border.width: 1
                                                            border.color: "#d1d5db"

                                                            Text {
                                                                anchors.centerIn: parent
                                                                text: "▲"
                                                                font.pixelSize: 8
                                                                color: "#374151"
                                                            }

                                                            MouseArea {
                                                                id: upStartHourMouseArea
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
                                                            width: 16
                                                            height: 12
                                                            color: downStartHourMouseArea.containsMouse ? "#e5e7eb" : "#f3f4f6"
                                                            radius: 4
                                                            border.width: 1
                                                            border.color: "#d1d5db"

                                                            Text {
                                                                anchors.centerIn: parent
                                                                text: "▼"
                                                                font.pixelSize: 8
                                                                color: "#374151"
                                                            }

                                                            MouseArea {
                                                                id: downStartHourMouseArea
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

                                            Text {
                                                text: "–"
                                                font.pixelSize: 16
                                                color: "#6b7280"
                                            }

                                            // End Time
                                            Rectangle {
                                                Layout.preferredWidth: 80
                                                Layout.preferredHeight: 36
                                                color: "#ffffff"
                                                radius: 8
                                                border.width: 1
                                                border.color: "#d1d5db"

                                                RowLayout {
                                                    anchors.fill: parent
                                                    anchors.margins: 4
                                                    spacing: 2

                                                    Text {
                                                        Layout.fillWidth: true
                                                        text: String(endHour).padStart(2, '0') + ":00"
                                                        font.pixelSize: 13
                                                        color: "#1f2937"
                                                        horizontalAlignment: Text.AlignHCenter
                                                        verticalAlignment: Text.AlignVCenter
                                                    }

                                                    Column {
                                                        Layout.preferredWidth: 16
                                                        spacing: 1

                                                        Rectangle {
                                                            width: 16
                                                            height: 12
                                                            color: upEndHourMouseArea.containsMouse ? "#e5e7eb" : "#f3f4f6"
                                                            radius: 4
                                                            border.width: 1
                                                            border.color: "#d1d5db"

                                                            Text {
                                                                anchors.centerIn: parent
                                                                text: "▲"
                                                                font.pixelSize: 8
                                                                color: "#374151"
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
                                                            width: 16
                                                            height: 12
                                                            color: downEndHourMouseArea.containsMouse ? "#e5e7eb" : "#f3f4f6"
                                                            radius: 4
                                                            border.width: 1
                                                            border.color: "#d1d5db"

                                                            Text {
                                                                anchors.centerIn: parent
                                                                text: "▼"
                                                                font.pixelSize: 8
                                                                color: "#374151"
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

                                            // Building Field
                                            Rectangle {
                                                Layout.preferredWidth: 100
                                                Layout.preferredHeight: 36
                                                color: "#ffffff"
                                                radius: 8
                                                border.width: 1
                                                border.color: buildingField.focus ? "#3b82f6" : "#d1d5db"

                                                TextField {
                                                    id: buildingField
                                                    anchors.fill: parent
                                                    text: building
                                                    placeholderText: "Building"
                                                    placeholderTextColor: "#9CA3AF"
                                                    background: Rectangle {
                                                        color: "transparent"
                                                    }
                                                    color: "#374151"
                                                    font.pixelSize: 13
                                                    leftPadding: 8
                                                    rightPadding: 8
                                                    topPadding: 8
                                                    bottomPadding: 8

                                                    onTextChanged: {
                                                        sessionsModel.setProperty(index, "building", text)
                                                    }
                                                }
                                            }

                                            // Room Field
                                            Rectangle {
                                                Layout.preferredWidth: 80
                                                Layout.preferredHeight: 36
                                                color: "#ffffff"
                                                radius: 8
                                                border.width: 1
                                                border.color: roomField.focus ? "#3b82f6" : "#d1d5db"

                                                TextField {
                                                    id: roomField
                                                    anchors.fill: parent
                                                    text: room
                                                    placeholderText: "Room"
                                                    placeholderTextColor: "#9CA3AF"
                                                    background: Rectangle {
                                                        color: "transparent"
                                                    }
                                                    color: "#374151"
                                                    font.pixelSize: 13
                                                    leftPadding: 8
                                                    rightPadding: 8
                                                    topPadding: 8
                                                    bottomPadding: 8

                                                    onTextChanged: {
                                                        sessionsModel.setProperty(index, "room", text)
                                                    }
                                                }
                                            }

                                            Item {
                                                Layout.fillWidth: true
                                            }

                                            // Delete Session Button
                                            Button {
                                                Layout.preferredWidth: 32
                                                Layout.preferredHeight: 32
                                                enabled: sessionsModel.count > 1

                                                background: Rectangle {
                                                    color: parent.enabled ? (deleteSessionMouseArea.containsMouse ? "#fee2e2" : "#f8fafc") : "#f3f4f6"
                                                    radius: 16
                                                    border.width: 1
                                                    border.color: parent.enabled ? "#e5e7eb" : "#d1d5db"
                                                }

                                                contentItem: Text {
                                                    text: "×"
                                                    font.pixelSize: 16
                                                    font.bold: true
                                                    color: parent.enabled ? "#ef4444" : "#d1d5db"
                                                    horizontalAlignment: Text.AlignHCenter
                                                    verticalAlignment: Text.AlignVCenter
                                                }

                                                MouseArea {
                                                    id: deleteSessionMouseArea
                                                    anchors.fill: parent
                                                    hoverEnabled: true
                                                    onClicked: {
                                                        if (sessionsModel.count > 1) {
                                                            sessionsModel.remove(index)
                                                        } else {
                                                            showError("Each group must have at least one session")
                                                        }
                                                    }
                                                    cursorShape: Qt.PointingHandCursor
                                                }
                                            }
                                        }
                                    }
                                }

                                // Add Session Button
                                Button {
                                    width: parent.width
                                    height: 44

                                    background: Rectangle {
                                        color: addSessionMouseArea.containsMouse ? "#f1f5f9" : "#f8fafc"
                                        border.color: "#e2e8f0"
                                        border.width: 2
                                        radius: 12
                                    }

                                    contentItem: RowLayout {
                                        spacing: 8
                                        anchors.centerIn: parent

                                        Text {
                                            text: "+"
                                            font.pixelSize: 16
                                            font.bold: true
                                            color: "#64748b"
                                        }

                                        Text {
                                            text: "Add Session"
                                            font.pixelSize: 14
                                            font.bold: true
                                            color: "#64748b"
                                        }
                                    }

                                    MouseArea {
                                        id: addSessionMouseArea
                                        anchors.fill: parent
                                        hoverEnabled: true
                                        onClicked: {
                                            sessionsModel.append({
                                                "day": "Sunday",
                                                "startHour": 9,
                                                "endHour": 10,
                                                "building": "",
                                                "room": ""
                                            })
                                        }
                                        cursorShape: Qt.PointingHandCursor
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }
    }

    // Action Buttons
    Rectangle {
        anchors.bottom: parent.bottom
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.leftMargin: 24
        anchors.rightMargin: 24
        height: 60
        color: "transparent"

        RowLayout {
            anchors.fill: parent
            spacing: 16

            Button {
                Layout.fillWidth: true
                Layout.preferredHeight: 50

                background: Rectangle {
                    color: cancelMouseArea.containsMouse ? "#f1f5f9" : "#ffffff"
                    radius: 12
                    border.width: 2
                    border.color: "#e2e8f0"
                }

                contentItem: Text {
                    text: "Cancel"
                    font.pixelSize: 16
                    font.bold: true
                    color: "#64748b"
                    horizontalAlignment: Text.AlignHCenter
                    verticalAlignment: Text.AlignVCenter
                }

                MouseArea {
                    id: cancelMouseArea
                    anchors.fill: parent
                    hoverEnabled: true
                    onClicked: {
                        resetPopup()
                        addCoursePopup.close()
                    }
                    cursorShape: Qt.PointingHandCursor
                }
            }

            Button {
                Layout.fillWidth: true
                Layout.preferredHeight: 50

                background: Rectangle {
                    color: saveMouseArea.containsMouse ? "#3b82f6" : "#4f46e5"
                    radius: 12
                    border.width: 0

                    // Gradient effect
                    Rectangle {
                        anchors.fill: parent
                        color: "transparent"
                        radius: 12
                        border.width: 1
                        border.color: "#ffffff30"
                    }
                }

                contentItem: Text {
                    text: "Save Course"
                    font.pixelSize: 16
                    font.bold: true
                    color: "#ffffff"
                    horizontalAlignment: Text.AlignHCenter
                    verticalAlignment: Text.AlignVCenter
                }

                MouseArea {
                    id: saveMouseArea
                    anchors.fill: parent
                    hoverEnabled: true
                    onClicked: {
                        // Validate required fields
                        if (courseNameField.text.trim() === "") {
                            showError("Course name is required")
                            return;
                        }

                        if (courseIdField.text.trim() === "") {
                            showError("Course ID is required")
                            return;
                        }

                        if (teacherField.text.trim() === "") {
                            showError("Teacher name is required")
                            return;
                        }

                        // Validate that course ID is a number
                        if (isNaN(parseInt(courseIdField.text))) {
                            showError("Course ID must be a valid number")
                            return;
                        }

                        // Validate that course ID is exactly 5 digits
                        if (courseIdField.text.length !== 5) {
                            showError("Course ID must be exactly 5 digits")
                            return;
                        }

                        // Collect session groups data and validate building/room fields
                        var sessionGroups = [];
                        var hasLecture = false;

                        // Iterate through all groups in the repeater
                        for (var groupIndex = 0; groupIndex < groupsModel.count; groupIndex++) {
                            var groupItem = groupsModel.get(groupIndex);

                            // Check if this group is a lecture
                            if (groupItem.groupType === "Lecture") {
                                hasLecture = true;
                            }

                            // Get the corresponding group delegate
                            var groupDelegate = groupsRepeater.itemAt(groupIndex);
                            if (!groupDelegate) {
                                continue;
                            }

                            // Find the ComboBox to get the actual selected type
                            var actualGroupType = groupItem.groupType; // Default fallback
                            function findComboBox(parent) {
                                for (var i = 0; i < parent.children.length; i++) {
                                    var child = parent.children[i];
                                    if (child.toString().indexOf("ComboBox") > -1 && child.currentText) {
                                        return child.currentText;
                                    }
                                    if (child.children && child.children.length > 0) {
                                        var found = findComboBox(child);
                                        if (found) return found;
                                    }
                                }
                                return null;
                            }

                            var comboBoxType = findComboBox(groupDelegate);
                            if (comboBoxType) {
                                actualGroupType = comboBoxType;
                                // Update hasLecture check with actual type
                                if (actualGroupType === "Lecture") {
                                    hasLecture = true;
                                }
                            }

                            // Access the sessionsModel through the delegate's children
                            var sessionsRepeater = null;

                            // Look for the sessions repeater in the group delegate
                            function findSessionsRepeater(parent) {
                                for (var i = 0; i < parent.children.length; i++) {
                                    var child = parent.children[i];
                                    if (child.toString().indexOf("Repeater") > -1 && child.model) {
                                        // Check if this repeater has session data
                                        if (child.model.count !== undefined) {
                                            return child;
                                        }
                                    }
                                    if (child.children && child.children.length > 0) {
                                        var found = findSessionsRepeater(child);
                                        if (found) return found;
                                    }
                                }
                                return null;
                            }

                            sessionsRepeater = findSessionsRepeater(groupDelegate);

                            if (!sessionsRepeater || !sessionsRepeater.model) {
                                continue;
                            }

                            var sessionsModel = sessionsRepeater.model;

                            // Collect sessions for this group and validate building/room
                            var groupSessions = [];
                            for (var sessionIndex = 0; sessionIndex < sessionsModel.count; sessionIndex++) {
                                var sessionData = sessionsModel.get(sessionIndex);

                                // Validate building and room fields
                                if (!sessionData.building || sessionData.building.trim() === "") {
                                    showError("All sessions must have a building specified (Group " + (groupIndex + 1) + ", Session " + (sessionIndex + 1) + ")")
                                    return;
                                }

                                if (!sessionData.room || sessionData.room.trim() === "") {
                                    showError("All sessions must have a room specified (Group " + (groupIndex + 1) + ", Session " + (sessionIndex + 1) + ")")
                                    return;
                                }

                                // Format the session data properly
                                var session = {
                                    day: sessionData.day,
                                    startTime: String(sessionData.startHour).padStart(2, '0') + ":00",
                                    endTime: String(sessionData.endHour).padStart(2, '0') + ":00",
                                    building: sessionData.building.trim(),
                                    room: sessionData.room.trim()
                                };

                                groupSessions.push(session);
                            }

                            var groupData = {
                                type: actualGroupType,
                                sessions: groupSessions
                            };

                            sessionGroups.push(groupData);
                        }

                        // Validate that course has at least one lecture group
                        if (!hasLecture) {
                            showError("A course must have at least one Lecture group")
                            return;
                        }

                        // Validate that there are session groups
                        if (sessionGroups.length === 0) {
                            showError("Please add at least one session group")
                            return;
                        }

                        // Emit the signal
                        courseCreated(courseNameField.text.trim(),
                            courseIdField.text.trim(),
                            teacherField.text.trim(),
                            sessionGroups);

                        // Reset and close
                        resetPopup()
                        addCoursePopup.close();
                    }
                    cursorShape: Qt.PointingHandCursor
                }
            }
        }
    }
}