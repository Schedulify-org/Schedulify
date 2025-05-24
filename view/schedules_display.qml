import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15
import Qt.labs.qmlmodels 1.0
import QtQuick.Controls.Basic
import "popups"
import "."

Page {
    id: schedulesDisplayPage

    background: Rectangle { color: "#ffffff" }

    property var controller: schedulesDisplayController
    property int currentIndex: controller.currentScheduleIndex
    property int totalSchedules: controller.getScheduleCount()
    property int numDays: 7

    // Minimum constraints
    property real minDayColumnWidth: 120
    property real minTimeColumnWidth: 80
    property real minRowHeight: 50
    property int minTextSize: 11

    // Table dimensions with dynamic calculations
    property real timeColumnWidth: minTimeColumnWidth
    property real dayColumnWidth: {
        var availableWidth = mainContent.width - 30; // Account for margins
        return Math.max(minDayColumnWidth, (availableWidth - timeColumnWidth) / numDays);
    }
    property int numberOfTimeSlots: 12
    property real headerHeight: 40
    property real uniformRowHeight: {
        var availableHeight = mainContent.height - topButtonsRow.height - 40;
        var availableTableHeight = availableHeight - headerHeight;
        return Math.max(minRowHeight, availableTableHeight / numberOfTimeSlots);
    }

    // Dynamic text size based on cell dimensions
    property real dynamicTextSize: Math.max(minTextSize,
        Math.min(15, Math.min(dayColumnWidth / 11.5, uniformRowHeight / 4.5)))

    // Header
    Rectangle {
        id: header
        width: parent.width
        height: 80
        color: "#ffffff"
        border.color: "#e5e7eb"

        Item {
            anchors {
                left: parent.left
                right: parent.right
                verticalCenter: parent.verticalCenter
            }
            height: coursesBackButton.height

            // Back Button
            Button {
                id: coursesBackButton
                width: 40
                height: 40
                anchors {
                    left: parent.left
                    leftMargin: 16
                    verticalCenter: parent.verticalCenter
                }
                background: Rectangle {
                    color: coursesBackMouseArea.containsMouse ? "#a8a8a8" : "#f3f4f6"
                    radius: 4
                }
                contentItem: Text {
                    text: "←"
                    font.pixelSize: 18
                    color: "#1f2937"
                    horizontalAlignment: Text.AlignHCenter
                    verticalAlignment: Text.AlignVCenter
                }
                MouseArea {
                    id: coursesBackMouseArea
                    anchors.fill: parent
                    hoverEnabled: true
                    onClicked: controller.goBack()
                    cursorShape: Qt.PointingHandCursor
                }

                ScrollBar.vertical: ScrollBar {
                    policy: scrollArea.contentHeight > scrollArea.height ? ScrollBar.AlwaysOn : ScrollBar.AsNeeded
                    active: scrollArea.contentHeight > scrollArea.height
                    visible: scrollArea.contentHeight > scrollArea.height
                }

                ScrollBar.horizontal: ScrollBar {
                    policy: scrollArea.contentWidth > scrollArea.width ? ScrollBar.AlwaysOn : ScrollBar.AsNeeded
                    active: scrollArea.contentWidth > scrollArea.width
                    visible: scrollArea.contentWidth > scrollArea.width
                }

                Component.onCompleted: {
                    Qt.callLater(function() {
                        var maxScroll = contentHeight - height;
                        if (contentY > maxScroll) {
                            contentY = maxScroll;
                        }
                        forceActiveFocus();
                    });
                }
            }

            // Screen Title
            Label {
                id: titleLabel
                text: "Generated schedules"
                font.pixelSize: 20
                color: "#1f2937"
                anchors {
                    left: coursesBackButton.right
                    leftMargin: 16
                    verticalCenter: parent.verticalCenter
                }
            }

            Button {
                id: logButtonC
                anchors {
                    right: parent.right
                    verticalCenter: parent.verticalCenter
                    rightMargin: 15
                }
                width: 40
                height: 40

                background: Rectangle {
                    color: logMouseArea.containsMouse ? "#a8a8a8" : "#f3f4f6"
                    radius: 10
                }

                // Custom content with SVG icon
                contentItem: Item {
                    anchors.fill: parent

                    // SVG Icon
                    Image {
                        id: logIcon
                        anchors.centerIn: parent
                        width: 24
                        height: 24
                        source: "qrc:/icons/ic-logs.svg"
                        sourceSize.width: 22
                        sourceSize.height: 22
                    }

                    // Hover tooltip
                    ToolTip {
                        id: logsTooltip
                        text: "Open Application Logs"
                        visible: logMouseArea.containsMouse
                        delay: 500
                        timeout: 3000

                        background: Rectangle {
                            color: "#374151"
                            radius: 4
                            border.color: "#4b5563"
                        }

                        contentItem: Text {
                            text: logsTooltip.text
                            color: "white"
                            font.pixelSize: 12
                        }
                    }
                }

                MouseArea {
                    id: logMouseArea
                    anchors.fill: parent
                    hoverEnabled: true
                    cursorShape: Qt.PointingHandCursor
                    onClicked: {
                        if (!logDisplayController.isLogWindowOpen) {
                            var component = Qt.createComponent("qrc:/log_display.qml");
                            if (component.status === Component.Ready) {
                                logDisplayController.setLogWindowOpen(true);
                                var logWindow = component.createObject(schedulesDisplayPage, {
                                    "onClosing": function(close) {
                                        logDisplayController.setLogWindowOpen(false);
                                    }
                                });
                                logWindow.show();
                            } else {
                                console.error("Error creating log window:", component.errorString());
                            }
                        }
                    }
                }
            }

            Button {
                id: exportButton
                width: 40
                height: 40
                anchors {
                    right: parent.right
                    rightMargin: 25 + logButtonC.width
                    verticalCenter: parent.verticalCenter
                }

                background: Rectangle {
                    color: exportMouseArea.containsMouse ? "#a8a8a8" : "#f3f4f6"
                    radius: 10
                }

                // Custom content with SVG icon
                contentItem: Item {
                    anchors.fill: parent

                    // SVG Icon
                    Image {
                        id: exportIcon
                        anchors.centerIn: parent
                        width: 24
                        height: 24
                        source: "qrc:/icons/ic-export.svg"
                        sourceSize.width: 22
                        sourceSize.height: 22
                    }

                    // Hover tooltip
                    ToolTip {
                        id: exportTooltip
                        text: "Export Schedule"
                        visible: exportMouseArea.containsMouse
                        delay: 500
                        timeout: 3000

                        background: Rectangle {
                            color: "#374151"
                            radius: 4
                            border.color: "#4b5563"
                        }

                        contentItem: Text {
                            text: exportTooltip.text
                            color: "white"
                            font.pixelSize: 12
                        }
                    }
                }

                MouseArea {
                    id: exportMouseArea
                    anchors.fill: parent
                    hoverEnabled: true
                    cursorShape: Qt.PointingHandCursor
                    onClicked: {
                        exportMenu.currentIndex = currentIndex
                        exportMenu.open()
                    }

                }
            }

            Button {
                id: preferenceButton
                width: 40
                height: 40
                anchors {
                    right: parent.right
                    rightMargin: 35 + exportButton.width + logButtonC.width
                    verticalCenter: parent.verticalCenter
                }

                background: Rectangle {
                    color: preferenceMouseArea.containsMouse ? "#a8a8a8" : "#f3f4f6"
                    radius: 10
                }

                // Custom content with SVG icon
                contentItem: Item {
                    anchors.fill: parent

                    // SVG Icon
                    Image {
                        id: preferenceIcon
                        anchors.centerIn: parent
                        width: 24
                        height: 24
                        source: "qrc:/icons/ic-preference.svg"
                        sourceSize.width: 22
                        sourceSize.height: 22
                    }

                    // Hover tooltip
                    ToolTip {
                        id: preferenceTooltip
                        text: "Set Schedule Preference"
                        visible: preferenceMouseArea.containsMouse
                        delay: 500
                        timeout: 3000

                        background: Rectangle {
                            color: "#374151"
                            radius: 4
                            border.color: "#4b5563"
                        }

                        contentItem: Text {
                            text: preferenceTooltip.text
                            color: "white"
                            font.pixelSize: 12
                        }
                    }
                }

                MouseArea {
                    id: preferenceMouseArea
                    anchors.fill: parent
                    hoverEnabled: true
                    cursorShape: Qt.PointingHandCursor
                    onClicked: preferenceMenu.open()
                }
            }
        }
    }

    // Export menu popup
    ExportMenu {
        id: exportMenu
        parent: Overlay.overlay

        // Connect the signals to local functions
        onPrintRequested: {
            console.log("Print requested for schedule", currentIndex + 1)
            if (controller) {
                controller.printScheduleDirectly()
            }
        }

        onSaveAsPngRequested: {
            console.log("Save as PNG requested for schedule", currentIndex + 1)
            if (schedulesDisplayController && tableContent) {
                schedulesDisplayController.captureAndSave(tableContent)
            }
        }

        onSaveAsCsvRequested: {
            console.log("Save as CSV requested for schedule", currentIndex + 1)
            if (controller) {
                controller.saveScheduleAsCSV()
            }
        }
    }

    // Preference menu popup
    PreferenceMenu {
        id: preferenceMenu
        parent: Overlay.overlay

        onSaveAndClose: {
            // Handle both filters and blocked times
            console.log("Filters:", filters)
            console.log("Blocked Times:", blockedTimes)
        }
    }

    // main content zone
    Rectangle{
        id: mainContent
        anchors {
            top: header.bottom
            left: parent.left
            right: parent.right
            bottom: footer.top
        }

        ColumnLayout {
            anchors.fill: parent
            anchors.margins: 10

            RowLayout {
                id: topButtonsRow
                width: parent.width
                Layout.fillWidth: true
                spacing: 10

                // ← הקודם
                Rectangle {
                    id: prevButton
                    radius: 4
                    color: prevMouseArea.containsMouse ? "#35455c" : "#1f2937"
                    implicitWidth: 50
                    implicitHeight: 40
                    visible: currentIndex > 0
                    Layout.alignment: Qt.AlignLeft

                    Text {
                        text: "←"
                        anchors.centerIn: parent
                        color: "white"
                        font.pixelSize: 20
                        font.bold: true
                    }

                    MouseArea {
                        id: prevMouseArea
                        anchors.fill: parent
                        hoverEnabled: true
                        cursorShape: Qt.PointingHandCursor
                        onClicked: controller.setCurrentScheduleIndex(currentIndex - 1)
                    }
                }

                Item {
                    Layout.fillWidth: true
                }

                ColumnLayout{
                    anchors.horizontalCenter: parent.horizontalCenter

                    Label {
                        text: "Schedule Options"
                        font.pixelSize: 20
                        color: "#3a3e45"
                        anchors.horizontalCenter: parent.horizontalCenter
                    }

                    Label {
                        text: "Schedule " + (currentIndex + 1) + " of " + totalSchedules
                        font.pixelSize: 15
                        color: "#3a3e45"
                        anchors.horizontalCenter: parent.horizontalCenter
                    }
                }

                Item {
                    Layout.fillWidth: true
                }

                // הבא →
                Rectangle {
                    id: nextButton
                    radius: 4
                    color: nextMouseArea.containsMouse ? "#35455c" : "#1f2937"
                    implicitWidth: 50
                    implicitHeight: 40
                    visible: currentIndex < totalSchedules - 1
                    Layout.alignment: Qt.AlignRight

                    Text {
                        text: "→"
                        anchors.centerIn: parent
                        color: "white"
                        font.pixelSize: 20
                        font.bold: true
                    }

                    MouseArea {
                        id: nextMouseArea
                        anchors.fill: parent
                        hoverEnabled: true
                        cursorShape: Qt.PointingHandCursor
                        onClicked: controller.setCurrentScheduleIndex(currentIndex + 1)
                    }
                }
            }

            // Scrollable table container - adapts to available space
            Flickable {
                id: scrollArea
                Layout.fillWidth: true
                Layout.fillHeight: true
                clip: true

                // Dynamic content sizing
                contentWidth: timeColumnWidth + (numDays * dayColumnWidth) + 30
                contentHeight: headerHeight + (numberOfTimeSlots * uniformRowHeight) + (numberOfTimeSlots - 1) + 10
                boundsBehavior: Flickable.StopAtBounds
                flickableDirection: Flickable.VerticalFlick

                Column  {
                    id: tableContent
                    width: scrollArea.contentWidth
                    spacing: 0

                    Row {
                        id: dayHeaderRow
                        height: headerHeight
                        spacing: 1
                        width: scrollArea.contentWidth

                        Rectangle {
                            width: timeColumnWidth
                            height: headerHeight
                            color: "#e5e7eb"
                            border.color: "#d1d5db"

                            Text {
                                anchors.centerIn: parent
                                text: "Hour/Day"
                                font.pixelSize: Math.max(minTextSize, dynamicTextSize)
                                font.bold: true
                                color: "#4b5563"
                            }
                        }

                        Repeater {
                            model: ["Sunday", "Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday"]

                            Rectangle {
                                width: dayColumnWidth
                                height: headerHeight
                                color: "#e5e7eb"
                                border.color: "#d1d5db"

                                Text {
                                    anchors.centerIn: parent
                                    text: modelData
                                    font.pixelSize: Math.max(minTextSize, dynamicTextSize)
                                    font.bold: true
                                    color: "#4b5563"
                                    wrapMode: Text.WordWrap
                                    elide: Text.ElideRight
                                }
                            }
                        }
                    }

                    TableView {
                        id: scheduleTable
                        width: scrollArea.contentWidth - 10
                        height: numberOfTimeSlots * uniformRowHeight + (numberOfTimeSlots - 1) + 20
                        clip: true
                        rowSpacing: 1
                        columnSpacing: 1
                        interactive: false

                        property var timeSlots: [
                            "8:00-9:00", "9:00-10:00", "10:00-11:00", "11:00-12:00",
                            "12:00-13:00", "13:00-14:00", "14:00-15:00", "15:00-16:00",
                            "16:00-17:00", "17:00-18:00", "18:00-19:00", "19:00-20:00"
                        ]

                        columnWidthProvider: function(col) {
                            return col === 0 ? timeColumnWidth : dayColumnWidth;
                        }

                        // Set uniform row height
                        rowHeightProvider: function(row) {
                            return uniformRowHeight;
                        }

                        model: TableModel {
                            TableModelColumn { display: "timeSlot" }
                            TableModelColumn { display: "sunday" }
                            TableModelColumn { display: "monday" }
                            TableModelColumn { display: "tuesday" }
                            TableModelColumn { display: "wednesday" }
                            TableModelColumn { display: "thursday" }
                            TableModelColumn { display: "friday" }
                            TableModelColumn { display: "saturday" }

                            rows: {
                                let rows = [];
                                const timeSlots = scheduleTable.timeSlots;
                                const days = ["sunday", "monday", "tuesday", "wednesday", "thursday", "friday", "saturday"];

                                for (let i = 0; i < timeSlots.length; i++) {
                                    let row = { timeSlot: timeSlots[i] };
                                    for (let day of days) {
                                        row[day] = "";
                                        row[day + "_type"] = "";
                                    }
                                    rows.push(row);
                                }

                                if (totalSchedules > 0) {
                                    for (let day = 0; day < 7; day++) {
                                        let dayName = days[day];
                                        let items = controller.getDayItems(currentIndex, day);

                                        for (let item of items) {
                                            let start = parseInt(item.start.split(":")[0]);
                                            let end = parseInt(item.end.split(":")[0]);

                                            for (let hour = start; hour < end; hour++) {
                                                for (let rowIndex = 0; rowIndex < timeSlots.length; rowIndex++) {
                                                    let slot = timeSlots[rowIndex];
                                                    let slotStart = parseInt(slot.split("-")[0].split(":")[0]);
                                                    let slotEnd = parseInt(slot.split("-")[1].split(":")[0]);

                                                    if (hour >= slotStart && hour < slotEnd) {
                                                        rows[rowIndex][dayName] +=
                                                            (rows[rowIndex][dayName] ? "\n\n" : "") +
                                                            "<b style='font-size:" + Math.max(minTextSize, dynamicTextSize - 1) + "px'>" + item.courseName + "</b> ("
                                                            + item.raw_id + ")" + "<br>" +
                                                            "Building: " + item.building + ", Room: " + item.room;

                                                        if (!rows[rowIndex][dayName + "_type"] ||
                                                            (item.type === "lecture") ||
                                                            (item.type === "lab" && rows[rowIndex][dayName + "_type"] === "tutorial")) {
                                                            rows[rowIndex][dayName + "_type"] = item.type;
                                                        }
                                                    }
                                                }
                                            }
                                        }
                                    }
                                }
                                return rows;
                            }
                        }

                        delegate: Rectangle {
                            implicitHeight: uniformRowHeight
                            border.width: 1
                            border.color: "#e0e0e0"
                            radius: 4

                            color: {
                                if (model.column === 0) {
                                    return "#d1d5db"; // Time slot column
                                }

                                if (!model.display || String(model.display).trim().length === 0) {
                                    return "#ffffff"; // Empty cell
                                }

                                // Get the type information for color coding
                                let columnName = "";
                                switch(model.column) {
                                    case 1: columnName = "sunday_type"; break;
                                    case 2: columnName = "monday_type"; break;
                                    case 3: columnName = "tuesday_type"; break;
                                    case 4: columnName = "wednesday_type"; break;
                                    case 5: columnName = "thursday_type"; break;
                                    case 6: columnName = "friday_type"; break;
                                    case 7: columnName = "saturday_type"; break;
                                }

                                let itemType = "";
                                if (columnName && model.row !== undefined) {
                                    // Access the type data from the model
                                    let rowData = parent.parent.model.rows[model.row];
                                    if (rowData && rowData[columnName]) {
                                        itemType = rowData[columnName];
                                    }
                                }

                                // Color based on item type
                                switch(itemType) {
                                    case "lecture": return "#b0e8ff";
                                    case "lab": return "#abffc6";
                                    case "tutorial": return "#edc8ff";
                                    default: return "#64748BFF";
                                }
                            }

                            Text {
                                anchors.fill: parent
                                wrapMode: Text.WordWrap
                                verticalAlignment: Text.AlignVCenter
                                horizontalAlignment: Text.AlignHCenter
                                padding: 4
                                font.pixelSize: Math.max(minTextSize, dynamicTextSize)
                                textFormat: Text.RichText
                                text: model.display ? String(model.display) : ""
                                color: "#000000"
                                clip: true
                                elide: Text.ElideRight
                            }
                        }
                    }
                }
            }
        }
    }

    // Footer
    Rectangle {
        id: footer
        width: parent.width
        height: 30
        anchors.bottom: parent.bottom
        color: "#ffffff"
        border.color: "#e5e7eb"

        Label {
            anchors.centerIn: parent
            text: "© 2025 Schedulify. All rights reserved."
            color: "#6b7280"
            font.pixelSize: 12
        }
    }
}