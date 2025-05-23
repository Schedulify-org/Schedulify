import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15
import Qt.labs.qmlmodels 1.0
import QtQuick.Controls.Basic
import "."

Page {
    id: schedulesDisplayPage

    background: Rectangle { color: "#ffffff" }

    property var controller: schedulesDisplayController
    property int currentIndex: controller.currentScheduleIndex
    property int totalSchedules: controller.getScheduleCount()
    property int numDays: 7
    property real dayColumnWidth: Math.max(135, (width - timeColumnWidth) / numDays)
    property real timeColumnWidth: 70

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
                    onClicked: exportMenu.open()

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
                        text: "Set schedule preference"
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
                    onClicked: exportMenu.open()

                }
            }
        }
    }

    // Popup export menu
    Popup {
        id: exportMenu
        width: 220
        height: menuColumn.height
        visible: false
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

        // Menu options column
        Column {
            id: menuColumn
            width: parent.width
            spacing: 0

            Rectangle {
                width: parent.width
                height: 40
                color: "transparent"

                Text {
                    text: "Export Schedule " + (currentIndex + 1)
                    font.pixelSize: 16
                    font.bold: true
                    anchors.centerIn: parent
                    color: "#ffffff"
                }
            }

            // Separator
            Rectangle {
                width: parent.width
                height: 1
                color: "#ffffff"
            }

            // Print option
            Rectangle {
                width: parent.width
                height: 50
                color: printOptionArea.containsMouse ? "#415263" : "transparent"

                Row {
                    anchors.centerIn: parent
                    spacing: 12

                    Text {
                        text: "🖨️"
                        font.pixelSize: 18
                        anchors.verticalCenter: parent.verticalCenter
                    }

                    Text {
                        text: "Print schedule"
                        font.pixelSize: 14
                        anchors.verticalCenter: parent.verticalCenter
                        color: "#ffffff"
                    }
                }

                MouseArea {
                    id: printOptionArea
                    anchors.fill: parent
                    hoverEnabled: true
                    cursorShape: Qt.PointingHandCursor
                    onClicked: {
                        controller.printScheduleDirectly()
                        exportMenu.close()
                    }
                }
            }

            // Separator
            Rectangle {
                width: parent.width
                height: 1
                color: "#ffffff"
            }

            // Save as PNG option
            Rectangle {
                width: parent.width
                height: 50
                color: pngOptionArea.containsMouse ? "#415263" : "transparent"

                Row {
                    anchors.centerIn: parent
                    spacing: 12

                    Text {
                        text: "🖼️"
                        font.pixelSize: 18
                        anchors.verticalCenter: parent.verticalCenter
                    }

                    Text {
                        text: "Save as PNG"
                        font.pixelSize: 14
                        anchors.verticalCenter: parent.verticalCenter
                        color: "#ffffff"
                    }
                }

                MouseArea {
                    id: pngOptionArea
                    anchors.fill: parent
                    hoverEnabled: true
                    cursorShape: Qt.PointingHandCursor
                    onClicked: {
                        schedulesDisplayController.captureAndSave(tableContent)
                        exportMenu.close()
                    }
                }
            }

            // Separator
            Rectangle {
                width: parent.width
                height: 1
                color: "#ffffff"
            }

            // Save as PDF option
            Rectangle {
                width: parent.width
                height: 50
                color: pdfOptionArea.containsMouse ? "#415263" : "transparent"

                Row {
                    anchors.centerIn: parent
                    spacing: 12

                    Text {
                        text: "📄"
                        font.pixelSize: 18
                        color: "#4b5563"
                    }

                    Text {
                        text: "Save as CSV"
                        font.pixelSize: 14
                        anchors.verticalCenter: parent.verticalCenter
                        color: "#ffffff"
                    }
                }

                MouseArea {
                    id: pdfOptionArea
                    anchors.fill: parent
                    hoverEnabled: true
                    cursorShape: Qt.PointingHandCursor
                    onClicked: {
                        controller.saveScheduleAsCSV()
                        exportMenu.close()
                    }
                }
            }

            // Separator
            Rectangle {
                width: parent.width
                height: 1
                color: "#ffffff"
            }

            // Cancel button
            Rectangle {
                width: parent.width
                height: 40
                color: abortOptionArea.containsMouse ? "#f18888" : "transparent"

                anchors {
                    left: parent.left
                    right: parent.right
                }

                Row {
                    anchors.centerIn: parent
                    spacing: 12

                    Text {
                        text: "❌"
                        font.pixelSize: 16
                        anchors.verticalCenter: parent.verticalCenter
                    }

                    Text {
                        text: "Cancel"
                        font.pixelSize: 14
                        font.bold: true
                        anchors.verticalCenter: parent.verticalCenter
                        color: "#df4646"
                    }
                }

                MouseArea {
                    id: abortOptionArea
                    anchors.fill: parent
                    hoverEnabled: true
                    cursorShape: Qt.PointingHandCursor
                    onClicked: {
                        exportMenu.close()
                    }
                }
            }

            // Separator
            Rectangle {
                width: parent.width
                height: 10
                color: "transparent"
            }
        }
    }

    MouseArea {
        id: mouseArea
        anchors.fill: parent
        propagateComposedEvents: true
        onClicked: {
            if (exportMenu.visible &&
                !exportButton.contains(exportButton.mapFromItem(mouseArea, mouse.x, mouse.y)) &&
                !exportMenu.contains(exportMenu.mapFromItem(mouseArea, mouse.x, mouse.y))) {
                exportMenu.visible = false
            }
            mouse.accepted = false
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

            Flickable {
                id: scrollArea
                Layout.fillWidth: true
                Layout.fillHeight: true
                clip: true

                contentWidth: timeColumnWidth + (numDays * dayColumnWidth) + 30
                contentHeight: dayHeaderRow.height + scheduleTable.height + 10
                boundsBehavior: Flickable.StopAtBounds
                flickableDirection: Flickable.HorizontalAndVerticalFlick

                Column  {
                    id: tableContent
                    width: scrollArea.contentWidth
                    spacing: 0

                    Row {
                        id: dayHeaderRow
                        height: 40
                        spacing: 1
                        width: parent.width

                        Rectangle {
                            width: timeColumnWidth
                            height: 40
                            color: "#e5e7eb"
                            border.color: "#d1d5db"

                            Text {
                                anchors.centerIn: parent
                                text: "Hour/Day"
                                font.pixelSize: 14
                                font.bold: true
                                color: "#4b5563"
                            }
                        }

                        Repeater {
                            model: ["Sunday", "Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday"]

                            Rectangle {
                                width: dayColumnWidth
                                height: 40
                                color: "#e5e7eb"
                                border.color: "#d1d5db"

                                Text {
                                    anchors.centerIn: parent
                                    text: modelData
                                    font.pixelSize: 14
                                    font.bold: true
                                    color: "#4b5563"
                                }
                            }
                        }
                    }

                    TableView {
                        id: scheduleTable
                        width: scrollArea.contentWidth - 10
                        height: timeSlots.length * 80 + 20
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
                                    for (let day of days) row[day] = "";
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
                                                            "<b style='font-size:13px'>" + item.courseName + "</b><br>" +
                                                            item.raw_id + " - " + item.type + "<br>" +
                                                            item.start + " - " + item.end + "<br>" +
                                                            "Building: " + item.building + ", Room: " + item.room;
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
                            implicitHeight: 80
                            border.width: 1
                            border.color: "#e0e0e0"
                            radius: 4

                            color: model.column === 0
                                ? "#d1d5db"
                                : (model.display && String(model.display).trim().length > 0
                                    ? "#64748BFF"
                                    : "#ffffff")

                            Text {
                                anchors.fill: parent
                                wrapMode: Text.WordWrap
                                verticalAlignment: Text.AlignVCenter
                                horizontalAlignment: Text.AlignHCenter
                                padding: 6
                                font.pixelSize: 11
                                textFormat: Text.RichText
                                text: model.display ? String(model.display) : ""
                                color: "#000000"
                            }
                        }

                    }
                }

                ScrollBar.vertical: ScrollBar {
                    policy: ScrollBar.AlwaysOn
                    active: true
                    visible: true
                }

                ScrollBar.horizontal: ScrollBar {
                    policy: ScrollBar.AlwaysOn
                    active: true
                    visible: true
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
        }
    }

    // Footer
    Rectangle {
        id: footer
        width: parent.width
        height: 60
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

