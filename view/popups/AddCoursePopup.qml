import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15
import QtQuick.Controls.Basic

Popup {
    id: addCoursePopup
    width: Math.min(parent.width * 0.9, 900)
    height: Math.min(parent.height * 0.95, 800)
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

    // Reset function to clear all data
    function resetPopup() {
        // Clear form fields
        courseNameField.text = ""
        courseIdField.text = ""
        teacherField.text = ""

        // Clear error message
        errorMessage = ""

        // Reset groups model to default state
        groupsModel.clear()
        groupsModel.append({
            "groupType": "Lecture",
            "groupIndex": 0
        })
    }

    // Show error message function
    function showError(message) {
        errorMessage = message
        errorTimer.restart()
    }

    // Timer to auto-hide error messages
    Timer {
        id: errorTimer
        interval: 4000 // 4 seconds
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
        border.color: "#d1d5db"
        border.width: 1
        radius: 8

        // Drop shadow effect (matching SlotBlockMenu)
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

    Item {
        id: contentContainer
        anchors {
            fill: parent
            margins: 20
        }

        // Header
        Item {
            id: headerSection
            anchors {
                top: parent.top
                left: parent.left
                right: parent.right
            }
            height: 40

            Text {
                id: headerTitle
                anchors {
                    left: parent.left
                    verticalCenter: parent.verticalCenter
                }
                text: "Add New Course"
                font.pixelSize: 20
                font.bold: true
                color: "#1f2937"
            }

            Button {
                id: closeButton
                anchors {
                    right: parent.right
                    verticalCenter: parent.verticalCenter
                }
                width: 30
                height: 30

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
            id: errorMessageContainer
            anchors {
                top: headerSection.bottom
                topMargin: 20
                left: parent.left
                right: parent.right
            }
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

        // Form Fields Section
        Item {
            id: formFieldsSection
            anchors {
                top: errorMessageContainer.bottom
                topMargin: errorMessage === "" ? 0 : 20
                left: parent.left
                right: parent.right
            }
            height: 220

            ColumnLayout {
                anchors.fill: parent
                spacing: 16

                // Course Name
                Item {
                    Layout.fillWidth: true
                    Layout.preferredHeight: 60

                    Label {
                        id: courseNameLabel
                        anchors {
                            top: parent.top
                            left: parent.left
                            right: parent.right
                        }
                        height: 22
                        text: "Course Name *"
                        font.pixelSize: 14
                        font.bold: true
                        color: "#374151"
                    }

                    Rectangle {
                        id: courseNameContainer
                        anchors {
                            top: courseNameLabel.bottom
                            topMargin: 8
                            left: parent.left
                            right: parent.right
                            bottom: parent.bottom
                        }
                        color: "#f9fafb"
                        radius: 6
                        border.width: 1
                        border.color: courseNameField.focus ? "#3b82f6" : "#d1d5db"

                        TextField {
                            id: courseNameField
                            anchors.fill: parent
                            placeholderText: "e.g., Introduction to Computer Science"
                            placeholderTextColor: "#9CA3AF"

                            background: Rectangle {
                                color: "transparent"
                                radius: 6
                            }

                            color: "#1f2937"
                            font.pixelSize: 14
                            leftPadding: 12
                            rightPadding: 12
                            topPadding: 8
                            bottomPadding: 8
                        }
                    }
                }

                // Course ID
                Item {
                    Layout.fillWidth: true
                    Layout.preferredHeight: 60

                    Label {
                        id: courseIdLabel
                        anchors {
                            top: parent.top
                            left: parent.left
                            right: parent.right
                        }
                        height: 22
                        text: "Course ID *"
                        font.pixelSize: 14
                        font.bold: true
                        color: "#374151"
                    }

                    Rectangle {
                        id: courseIdContainer
                        anchors {
                            top: courseIdLabel.bottom
                            topMargin: 8
                            left: parent.left
                            right: parent.right
                            bottom: parent.bottom
                        }
                        color: "#f9fafb"
                        radius: 6
                        border.width: 1
                        border.color: courseIdField.focus ? "#3b82f6" : "#d1d5db"

                        TextField {
                            id: courseIdField
                            anchors.fill: parent
                            placeholderText: "e.g., 12345"
                            placeholderTextColor: "#9CA3AF"
                            validator: RegularExpressionValidator { regularExpression: /[0-9]{0,5}/ }

                            background: Rectangle {
                                color: "transparent"
                                radius: 6
                            }

                            color: "#1f2937"
                            font.pixelSize: 14
                            leftPadding: 12
                            rightPadding: 12
                            topPadding: 8
                            bottomPadding: 8
                        }
                    }
                }

                // Teacher
                Item {
                    Layout.fillWidth: true
                    Layout.preferredHeight: 60

                    Label {
                        id: teacherLabel
                        anchors {
                            top: parent.top
                            left: parent.left
                            right: parent.right
                        }
                        height: 22
                        text: "Teacher *"
                        font.pixelSize: 14
                        font.bold: true
                        color: "#374151"
                    }

                    Rectangle {
                        id: teacherContainer
                        anchors {
                            top: teacherLabel.bottom
                            topMargin: 8
                            left: parent.left
                            right: parent.right
                            bottom: parent.bottom
                        }
                        color: "#f9fafb"
                        radius: 6
                        border.width: 1
                        border.color: teacherField.focus ? "#3b82f6" : "#d1d5db"

                        TextField {
                            id: teacherField
                            anchors.fill: parent
                            placeholderText: "e.g., Prof. Johnson"
                            placeholderTextColor: "#9CA3AF"

                            background: Rectangle {
                                color: "transparent"
                                radius: 6
                            }

                            color: "#1f2937"
                            font.pixelSize: 14
                            leftPadding: 12
                            rightPadding: 12
                            topPadding: 8
                            bottomPadding: 8
                        }
                    }
                }
            }
        }

        // Session Groups Section Header
        Item {
            id: sessionGroupsHeaderSection
            anchors {
                top: formFieldsSection.bottom
                topMargin: 20
                left: parent.left
                right: parent.right
            }
            height: 40

            Label {
                id: sessionGroupsTitle
                anchors {
                    left: parent.left
                    verticalCenter: parent.verticalCenter
                }
                text: "Session Groups"
                font.pixelSize: 16
                font.bold: true
                color: "#1f2937"
            }

            Button {
                id: addGroupButton
                anchors {
                    right: parent.right
                    verticalCenter: parent.verticalCenter
                }
                width: 120
                height: 32

                background: Rectangle {
                    color: addGroupMouseArea.containsMouse ? "#f3f4f6" : "#ffffff"
                    radius: 6
                    border.width: 1
                    border.color: "#d1d5db"
                }

                contentItem: RowLayout {
                    spacing: 6
                    anchors.centerIn: parent

                    Text {
                        text: "+"
                        font.pixelSize: 14
                        color: "#374151"
                    }

                    Text {
                        text: "Add Group"
                        font.pixelSize: 12
                        color: "#374151"
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

        // Scrollable Groups List
        ScrollView {
            id: groupsScrollView
            anchors {
                top: sessionGroupsHeaderSection.bottom
                topMargin: 12
                left: parent.left
                right: parent.right
                bottom: actionButtonsSection.top
                bottomMargin: 20
            }
            clip: true
            contentWidth: availableWidth

            Column {
                width: groupsScrollView.width
                spacing: 16

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
                        border.color: "#e5e7eb"
                        border.width: 1
                        radius: 8
                        height: groupContent.implicitHeight + 24

                        property alias groupType: groupTypeCombo.currentText
                        property var sessionsList: []

                        ColumnLayout {
                            id: groupContent
                            anchors.fill: parent
                            anchors.margins: 12
                            spacing: 12

                            // Group Header
                            RowLayout {
                                Layout.fillWidth: true

                                RowLayout {
                                    spacing: 12

                                    ComboBox {
                                        id: groupTypeCombo
                                        model: ["Lecture", "Tutorial", "Lab"]
                                        currentIndex: {
                                            switch(groupType) {
                                                case "Tutorial": return 1
                                                case "Lab": return 2
                                                default: return 0
                                            }
                                        }

                                        background: Rectangle {
                                            color: "#f9fafb"
                                            border.color: "#d1d5db"
                                            border.width: 1
                                            radius: 6
                                        }

                                        contentItem: Text {
                                            text: parent.currentText
                                            font.pixelSize: 14
                                            color: "#1f2937"
                                            leftPadding: 12
                                            verticalAlignment: Text.AlignVCenter
                                        }

                                        // Update the model when selection changes
                                        onCurrentTextChanged: {
                                            groupsModel.setProperty(index, "groupType", currentText)
                                        }
                                    }

                                    Text {
                                        text: "Group " + (groupIndex + 1)
                                        font.pixelSize: 14
                                        color: "#6b7280"
                                    }
                                }

                                Item { Layout.fillWidth: true }

                                Button {
                                    Layout.preferredWidth: 28
                                    Layout.preferredHeight: 28
                                    enabled: groupsModel.count > 1

                                    background: Rectangle {
                                        color: parent.enabled ? (deleteGroupMouseArea.containsMouse ? "#fee2e2" : "transparent") : "#f3f4f6"
                                        radius: 14
                                    }

                                    contentItem: Text {
                                        text: "×"
                                        font.pixelSize: 16
                                        color: parent.enabled ? "#6b7280" : "#d1d5db"
                                        horizontalAlignment: Text.AlignHCenter
                                        verticalAlignment: Text.AlignVCenter
                                    }

                                    MouseArea {
                                        id: deleteGroupMouseArea
                                        anchors.fill: parent
                                        hoverEnabled: true
                                        onClicked: {
                                            if (groupsModel.count > 1) {
                                                groupsModel.remove(index)
                                                // Update group indices
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

                            // Sessions for this group
                            Column {
                                Layout.fillWidth: true
                                spacing: 8

                                Repeater {
                                    id: sessionsRepeater
                                    model: ListModel {
                                        id: sessionsModel
                                        Component.onCompleted: {
                                            // Add one default session
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
                                        color: "#f9fafb"
                                        border.color: "#e5e7eb"
                                        border.width: 1
                                        radius: 6
                                        height: 40

                                        // Time validation function for this session
                                        function validateTimes() {
                                            if (endHour <= startHour) {
                                                endHour = startHour + 1;
                                                if (endHour > 23) {
                                                    endHour = 23;
                                                    startHour = 22;
                                                }
                                            }
                                            // Update the model
                                            sessionsModel.setProperty(index, "startHour", startHour)
                                            sessionsModel.setProperty(index, "endHour", endHour)
                                        }

                                        // Single row: Day, Times, Building, Room, Delete button
                                        RowLayout {
                                            anchors.fill: parent
                                            anchors.margins: 8
                                            spacing: 6

                                            ComboBox {
                                                id: dayCombo
                                                Layout.preferredWidth: 85
                                                model: ["Sunday", "Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday"]
                                                currentIndex: {
                                                    var days = ["Sunday", "Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday"]
                                                    return Math.max(0, days.indexOf(day))
                                                }

                                                background: Rectangle {
                                                    color: "#ffffff"
                                                    border.color: "#d1d5db"
                                                    border.width: 1
                                                    radius: 4
                                                }

                                                contentItem: Text {
                                                    text: parent.currentText
                                                    font.pixelSize: 14
                                                    color: "#374151"
                                                    leftPadding: 6
                                                    verticalAlignment: Text.AlignVCenter
                                                }

                                                onCurrentTextChanged: {
                                                    sessionsModel.setProperty(index, "day", currentText)
                                                }
                                            }

                                            // Start Time Controls
                                            Rectangle {
                                                Layout.preferredWidth: 65
                                                Layout.preferredHeight: 28
                                                color: "#ffffff"
                                                radius: 4
                                                border.width: 1
                                                border.color: "#d1d5db"

                                                Item {
                                                    anchors.centerIn: parent
                                                    width: 55
                                                    height: parent.height

                                                    // Hour display
                                                    Text {
                                                        anchors {
                                                            left: parent.left
                                                            leftMargin: 3
                                                            verticalCenter: parent.verticalCenter
                                                        }
                                                        width: 32
                                                        text: String(startHour).padStart(2, '0') + ":00"
                                                        font.pixelSize: 14
                                                        color: "#1f2937"
                                                        horizontalAlignment: Text.AlignHCenter
                                                    }

                                                    // Hour controls
                                                    Item {
                                                        anchors {
                                                            right: parent.right
                                                            rightMargin: 2
                                                            verticalCenter: parent.verticalCenter
                                                        }
                                                        width: 16
                                                        height: 22

                                                        Rectangle {
                                                            anchors {
                                                                top: parent.top
                                                                left: parent.left
                                                                right: parent.right
                                                            }
                                                            height: 10
                                                            color: upStartHourMouseArea.containsMouse ? "#e5e7eb" : "#f3f4f6"
                                                            radius: 2
                                                            border.width: 1
                                                            border.color: "#d1d5db"

                                                            Text {
                                                                anchors.centerIn: parent
                                                                text: "▲"
                                                                font.pixelSize: 7
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
                                                            anchors {
                                                                bottom: parent.bottom
                                                                left: parent.left
                                                                right: parent.right
                                                            }
                                                            height: 10
                                                            color: downStartHourMouseArea.containsMouse ? "#e5e7eb" : "#f3f4f6"
                                                            radius: 2
                                                            border.width: 1
                                                            border.color: "#d1d5db"

                                                            Text {
                                                                anchors.centerIn: parent
                                                                text: "▼"
                                                                font.pixelSize: 7
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
                                                text: "-"
                                                font.pixelSize: 14
                                                color: "#6b7280"
                                            }

                                            // End Time Controls
                                            Rectangle {
                                                Layout.preferredWidth: 65
                                                Layout.preferredHeight: 28
                                                color: "#ffffff"
                                                radius: 4
                                                border.width: 1
                                                border.color: "#d1d5db"

                                                Item {
                                                    anchors.centerIn: parent
                                                    width: 55
                                                    height: parent.height

                                                    // Hour display
                                                    Text {
                                                        anchors {
                                                            left: parent.left
                                                            leftMargin: 3
                                                            verticalCenter: parent.verticalCenter
                                                        }
                                                        width: 32
                                                        text: String(endHour).padStart(2, '0') + ":00"
                                                        font.pixelSize: 14
                                                        color: "#1f2937"
                                                        horizontalAlignment: Text.AlignHCenter
                                                    }

                                                    // Hour controls
                                                    Item {
                                                        anchors {
                                                            right: parent.right
                                                            rightMargin: 2
                                                            verticalCenter: parent.verticalCenter
                                                        }
                                                        width: 16
                                                        height: 22

                                                        Rectangle {
                                                            anchors {
                                                                top: parent.top
                                                                left: parent.left
                                                                right: parent.right
                                                            }
                                                            height: 10
                                                            color: upEndHourMouseArea.containsMouse ? "#e5e7eb" : "#f3f4f6"
                                                            radius: 2
                                                            border.width: 1
                                                            border.color: "#d1d5db"

                                                            Text {
                                                                anchors.centerIn: parent
                                                                text: "▲"
                                                                font.pixelSize: 7
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
                                                            anchors {
                                                                bottom: parent.bottom
                                                                left: parent.left
                                                                right: parent.right
                                                            }
                                                            height: 10
                                                            color: downEndHourMouseArea.containsMouse ? "#e5e7eb" : "#f3f4f6"
                                                            radius: 2
                                                            border.width: 1
                                                            border.color: "#d1d5db"

                                                            Text {
                                                                anchors.centerIn: parent
                                                                text: "▼"
                                                                font.pixelSize: 7
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

                                            TextField {
                                                id: buildingField
                                                Layout.preferredWidth: 70
                                                Layout.preferredHeight: 28
                                                text: building
                                                placeholderText: "Building"
                                                placeholderTextColor: "#9CA3AF"

                                                background: Rectangle {
                                                    color: "#ffffff"
                                                    border.color: "#d1d5db"
                                                    border.width: 1
                                                    radius: 4
                                                }

                                                color: "#374151"
                                                font.pixelSize: 14
                                                leftPadding: 6
                                                rightPadding: 6
                                                topPadding: 4
                                                bottomPadding: 4

                                                onTextChanged: {
                                                    sessionsModel.setProperty(index, "building", text)
                                                }
                                            }

                                            TextField {
                                                id: roomField
                                                Layout.preferredWidth: 60
                                                Layout.preferredHeight: 28
                                                text: room
                                                placeholderText: "Room"
                                                placeholderTextColor: "#9CA3AF"

                                                background: Rectangle {
                                                    color: "#ffffff"
                                                    border.color: "#d1d5db"
                                                    border.width: 1
                                                    radius: 4
                                                }

                                                color: "#374151"
                                                font.pixelSize: 14
                                                leftPadding: 6
                                                rightPadding: 6
                                                topPadding: 4
                                                bottomPadding: 4

                                                onTextChanged: {
                                                    sessionsModel.setProperty(index, "room", text)
                                                }
                                            }

                                            Item { Layout.fillWidth: true }

                                            Button {
                                                Layout.preferredWidth: 24
                                                Layout.preferredHeight: 24
                                                enabled: sessionsModel.count > 1

                                                background: Rectangle {
                                                    color: parent.enabled ? (deleteSessionMouseArea.containsMouse ? "#fee2e2" : "transparent") : "#f3f4f6"
                                                    radius: 12
                                                }

                                                contentItem: Text {
                                                    text: "×"
                                                    font.pixelSize: 14
                                                    color: parent.enabled ? "#6b7280" : "#d1d5db"
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
                                    height: 28

                                    background: Rectangle {
                                        color: addSessionMouseArea.containsMouse ? "#f3f4f6" : "transparent"
                                        border.color: "#d1d5db"
                                        border.width: 1
                                        radius: 4
                                    }

                                    contentItem: RowLayout {
                                        spacing: 4
                                        anchors.centerIn: parent

                                        Text {
                                            text: "+"
                                            font.pixelSize: 12
                                            color: "#6b7280"
                                        }

                                        Text {
                                            text: "Add Session"
                                            font.pixelSize: 11
                                            color: "#6b7280"
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

        // Action buttons
        Item {
            id: actionButtonsSection
            anchors {
                bottom: parent.bottom
                left: parent.left
                right: parent.right
            }
            height: 45

            Button {
                id: cancelButton
                anchors {
                    left: parent.left
                    top: parent.top
                    bottom: parent.bottom
                }
                width: (parent.width - 12) / 2

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
                    onClicked: {
                        resetPopup()
                        addCoursePopup.close()
                    }
                    cursorShape: Qt.PointingHandCursor
                }
            }

            Button {
                id: saveButton
                anchors {
                    right: parent.right
                    top: parent.top
                    bottom: parent.bottom
                }
                width: (parent.width - 12) / 2

                background: Rectangle {
                    color: saveMouseArea.containsMouse ? "#1d4ed8" : "#2563eb"
                    radius: 6
                }

                contentItem: Text {
                    text: "Save Course"
                    font.pixelSize: 14
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