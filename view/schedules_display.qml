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
    property var scheduleModel: controller ? controller.scheduleModel : null
    property int currentIndex: scheduleModel ? scheduleModel.currentScheduleIndex : 0
    property int totalSchedules: scheduleModel ? scheduleModel.scheduleCount : 0
    property int numDays: 7

    // Minimum constraints
    property real minDayColumnWidth: 120
    property real minTimeColumnWidth: 80
    property real minRowHeight: 50
    property int minTextSize: 11

    // Table dimensions with dynamic calculations
    property real timeColumnWidth: minTimeColumnWidth
    property real dayColumnWidth: {
        var availableWidth = mainContent.width - 30;
        return Math.max(minDayColumnWidth, (availableWidth - timeColumnWidth) / numDays);
    }
    property int numberOfTimeSlots: 13
    property real headerHeight: 40
    property real uniformRowHeight: {
        var availableHeight = mainContent.height - topButtonsRow.height - 40;
        var availableTableHeight = availableHeight - headerHeight;
        return Math.max(minRowHeight, availableTableHeight / numberOfTimeSlots);
    }

    // Dynamic text size based on cell dimensions
    property real dynamicTextSize: Math.max(minTextSize,
        Math.min(15, Math.min(dayColumnWidth / 11.5, uniformRowHeight / 4.5)))

    // Listen to model changes
    Connections {
        target: scheduleModel
        function onCurrentScheduleIndexChanged() {
            if (tableModel) {
                tableModel.updateRows()
            }
        }
        function onScheduleDataChanged() {
            if (tableModel) {
                tableModel.updateRows()
            }
        }
    }

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

                Component.onCompleted: {
                    forceActiveFocus();
                }
            }

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

                contentItem: Item {
                    anchors.fill: parent

                    Image {
                        id: logIcon
                        anchors.centerIn: parent
                        width: 24
                        height: 24
                        source: "qrc:/icons/ic-logs.svg"
                        sourceSize.width: 22
                        sourceSize.height: 22
                    }

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

                contentItem: Item {
                    anchors.fill: parent

                    Image {
                        id: exportIcon
                        anchors.centerIn: parent
                        width: 24
                        height: 24
                        source: "qrc:/icons/ic-export.svg"
                        sourceSize.width: 22
                        sourceSize.height: 22
                    }

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

                contentItem: Item {
                    anchors.fill: parent

                    Image {
                        id: preferenceIcon
                        anchors.centerIn: parent
                        width: 24
                        height: 24
                        source: "qrc:/icons/ic-preference.svg"
                        sourceSize.width: 22
                        sourceSize.height: 22
                    }

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

    ExportMenu {
        id: exportMenu
        parent: Overlay.overlay

        onPrintRequested: {
            if (controller) {
                controller.printScheduleDirectly()
            }
        }

        onSaveAsPngRequested: {
            if (schedulesDisplayController && tableContent) {
                schedulesDisplayController.captureAndSave(tableContent)
            }
        }

        onSaveAsCsvRequested: {
            if (controller) {
                controller.saveScheduleAsCSV()
            }
        }
    }

    MainPreferencePopup {
        id: preferenceMenu
        parent: Overlay.overlay

        // ADD THESE SIGNAL CONNECTIONS:
        onFiltersApplied: function(filterData, blockedTimes) {
            if (controller) {
                controller.applyFiltersAndBlockedTimes(filterData, blockedTimes)
            }
        }
    }

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

                Rectangle {
                    id: prevButton
                    radius: 4

                    property bool isEnabled: scheduleModel ? scheduleModel.canGoPrevious : false

                    color: {
                        if (!isEnabled) return "#e5e7eb";
                        return prevMouseArea.containsMouse ? "#35455c" : "#1f2937";
                    }

                    implicitWidth: 50
                    implicitHeight: 40
                    Layout.alignment: Qt.AlignLeft
                    opacity: isEnabled ? 1.0 : 0.5

                    Text {
                        text: "←"
                        anchors.centerIn: parent
                        color: parent.isEnabled ? "white" : "#9ca3af"
                        font.pixelSize: 20
                        font.bold: true
                    }

                    MouseArea {
                        id: prevMouseArea
                        anchors.fill: parent
                        hoverEnabled: parent.isEnabled
                        cursorShape: parent.isEnabled ? Qt.PointingHandCursor : Qt.ForbiddenCursor
                        enabled: parent.isEnabled
                        onClicked: {
                            if (scheduleModel) {
                                scheduleModel.previousSchedule()
                            }
                        }
                    }
                }

                Item {
                    Layout.fillWidth: true
                }

                ColumnLayout{
                    Layout.alignment: Qt.AlignHCenter

                    Label {
                        Layout.alignment: Qt.AlignHCenter
                        text: "Schedule Options"
                        font.pixelSize: 20
                        color: "#3a3e45"
                    }

                    Label {
                        Layout.alignment: Qt.AlignHCenter
                        text: "Schedule " + (currentIndex + 1) + " of " + totalSchedules
                        font.pixelSize: 15
                        color: "#3a3e45"
                    }
                }

                Item {
                    Layout.fillWidth: true
                }

                Rectangle {
                    id: nextButton
                    radius: 4

                    property bool isEnabled: scheduleModel ? scheduleModel.canGoNext : false

                    color: {
                        if (!isEnabled) return "#e5e7eb";
                        return nextMouseArea.containsMouse ? "#35455c" : "#1f2937";
                    }

                    implicitWidth: 50
                    implicitHeight: 40
                    Layout.alignment: Qt.AlignRight
                    opacity: isEnabled ? 1.0 : 0.5

                    Text {
                        text: "→"
                        anchors.centerIn: parent
                        color: parent.isEnabled ? "white" : "#9ca3af"
                        font.pixelSize: 20
                        font.bold: true
                    }

                    MouseArea {
                        id: nextMouseArea
                        anchors.fill: parent
                        hoverEnabled: parent.isEnabled
                        cursorShape: parent.isEnabled ? Qt.PointingHandCursor : Qt.ForbiddenCursor
                        enabled: parent.isEnabled
                        onClicked: {
                            if (scheduleModel) {
                                scheduleModel.nextSchedule()
                            }
                        }
                    }
                }
            }

            Flickable {
                id: scrollArea
                Layout.fillWidth: true
                Layout.fillHeight: true
                clip: true

                contentWidth: timeColumnWidth + (numDays * dayColumnWidth) + 30
                contentHeight: headerHeight + (numberOfTimeSlots * uniformRowHeight) + (numberOfTimeSlots - 1) + 2
                boundsBehavior: Flickable.StopAtBounds
                flickableDirection: Flickable.VerticalFlick

                Column  {
                    id: tableContent
                    width: scrollArea.contentWidth
                    spacing: 1

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
                            radius: 4

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
                                radius: 4

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
                            "16:00-17:00", "17:00-18:00", "18:00-19:00", "19:00-20:00",
                            "20:00-21:00"
                        ]

                        columnWidthProvider: function(col) {
                            return col === 0 ? timeColumnWidth : dayColumnWidth;
                        }

                        rowHeightProvider: function(row) {
                            return uniformRowHeight;
                        }

                        model: TableModel {
                            id: tableModel
                            TableModelColumn { display: "timeSlot" }
                            TableModelColumn { display: "sunday" }
                            TableModelColumn { display: "monday" }
                            TableModelColumn { display: "tuesday" }
                            TableModelColumn { display: "wednesday" }
                            TableModelColumn { display: "thursday" }
                            TableModelColumn { display: "friday" }
                            TableModelColumn { display: "saturday" }

                            function updateRows() {
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

                                if (totalSchedules > 0 && scheduleModel) {
                                    for (let day = 0; day < 7; day++) {
                                        let dayName = days[day];
                                        let items = scheduleModel.getCurrentDayItems(day);

                                        for (let itemIndex = 0; itemIndex < items.length; itemIndex++) {
                                            let item = items[itemIndex];
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
                                tableModel.rows = rows;
                            }

                            Component.onCompleted: updateRows()
                        }

                        delegate: Rectangle {
                            implicitHeight: uniformRowHeight
                            border.width: 1
                            border.color: "#e0e0e0"
                            radius: 4

                            property string columnName: {
                                switch(model.column) {
                                    case 1: return "sunday_type";
                                    case 2: return "monday_type";
                                    case 3: return "tuesday_type";
                                    case 4: return "wednesday_type";
                                    case 5: return "thursday_type";
                                    case 6: return "friday_type";
                                    case 7: return "saturday_type";
                                    default: return "";
                                }
                            }

                            property string itemType: {
                                if (columnName && model.row !== undefined) {
                                    let rowData = parent.parent.model.rows[model.row];
                                    if (rowData && rowData[columnName]) {
                                        return rowData[columnName];
                                    }
                                }
                                return "";
                            }

                            color: {
                                if (model.column === 0) {
                                    return "#d1d5db";
                                }

                                if (!model.display || String(model.display).trim().length === 0) {
                                    return "#ffffff";
                                }

                                switch(itemType) {
                                    case "lecture": return "#b0e8ff";
                                    case "lab": return "#abffc6";
                                    case "tutorial": return "#edc8ff";
                                    default: return "#64748BFF";
                                }
                            }

                            ToolTip {
                                id: sessionTooltip
                                text: itemType || "No session type"
                                visible: sessionMouseArea.containsMouse && itemType !== ""
                                delay: 500
                                timeout: 3000

                                background: Rectangle {
                                    color: "#374151"
                                    radius: 4
                                    border.color: "#4b5563"
                                }

                                contentItem: Text {
                                    text: sessionTooltip.text
                                    color: "white"
                                    font.pixelSize: 12
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

                            MouseArea {
                                id: sessionMouseArea
                                anchors.fill: parent
                                hoverEnabled: true
                            }
                        }
                    }
                }
            }
        }
    }

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