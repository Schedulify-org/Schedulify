import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15
import Qt.labs.qmlmodels 1.0

Page {
    id: schedulesDisplayPage
    background: Rectangle { color: "#ffffff" }

    property var controller: schedulesDisplayController
    property int currentIndex: controller.currentScheduleIndex
    property int totalSchedules: controller.getScheduleCount()
    // מספר הימים ורוחב כל עמודה
    property int numDays: 7
    property real dayColumnWidth: Math.max(135, (width - timeColumnWidth) / numDays)
    property real timeColumnWidth: 70

    ColumnLayout {
        anchors.fill: parent
        spacing: 0

        // --- Header ---
        Rectangle {
            id: header
            Layout.fillWidth: true
            height: 80
            color: "#ffffff"
            border.color: "#e5e7eb"

            Row {
                anchors.verticalCenter: parent.verticalCenter
                spacing: 16
                padding: 16

                Button {
                    width: 40
                    height: 40
                    background: Rectangle {
                        color: "#f3f4f6"
                        radius: 4
                    }
                    contentItem: Text {
                        text: "←"
                        font.pixelSize: 18
                        color: "#1f2937"
                        horizontalAlignment: Text.AlignHCenter
                        verticalAlignment: Text.AlignVCenter
                    }
                    onClicked: controller.goBack()
                }

                Label {
                    text: "Generated schedules"
                    font.pixelSize: 20
                    color: "#1f2937"
                    verticalAlignment: Text.AlignVCenter
                }
            }
        }

        Label {
            text: "מערכת נוכחית: " + (currentIndex + 1) + " / " + totalSchedules
            font.pixelSize: 20
            color: "#3a3e45"
            Layout.alignment: Qt.AlignHCenter
            padding: 12
        }

        Flickable {
            id: scrollArea
            Layout.fillWidth: true
            Layout.fillHeight: true
            clip: true
            contentWidth: timeColumnWidth + (numDays * dayColumnWidth)
            contentHeight: dayHeaderRow.height + scheduleTable.height
            flickableDirection: Flickable.HorizontalAndVerticalFlick

            Column {
                id: tableContent
                width: scrollArea.contentWidth

                // Days row
                Row {
                    id: dayHeaderRow
                    height: 40

                    Rectangle {
                        width: timeColumnWidth
                        height: 40
                        color: "#e5e7eb"
                        border.color: "#d1d5db"

                        Text {
                            anchors.centerIn: parent
                            text: "שעות"
                            font.pixelSize: 14
                            font.bold: true
                            color: "#4b5563"
                        }
                    }

                    Repeater {
                        model: ["ראשון", "שני", "שלישי", "רביעי", "חמישי", "שישי", "שבת"]

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

                // Table
                TableView {
                    id: scheduleTable
                    width: scrollArea.contentWidth
                    height: timeSlots.length * 80
                    clip: true
                    rowSpacing: 1
                    columnSpacing: 1
                    interactive: false // Let the parent Flickable handle scrolling

                    property var timeSlots: [
                        "8:00-9:00", "9:00-10:00", "10:00-11:00", "11:00-12:00",
                        "12:00-13:00", "13:00-14:00", "14:00-15:00", "15:00-16:00", "16:00-17:00",
                        "17:00-18:00", "18:00-19:00", "19:00-20:00"
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
                                                        "בניין: " + item.building + ", חדר: " + item.room;
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
                            ? "#f3f4f6"
                            : (model.display && String(model.display).trim().length > 0
                                ? "#e8ebf2"
                                : "#ffffff")

                        Text {
                            anchors.fill: parent
                            wrapMode: Text.WordWrap
                            verticalAlignment: Text.AlignVCenter
                            horizontalAlignment: Text.AlignLeft
                            padding: 6
                            font.pixelSize: 11
                            textFormat: Text.RichText
                            text: model.display ? String(model.display) : ""
                            color: "#2e2e2e"
                        }
                    }

                    // לאפס את רוחב העמודות כאשר גודל המסך משתנה
                    onWidthChanged: {
                        forceLayout();
                    }
                }
            }

            // Add scrollbars for better UX
            ScrollBar.vertical: ScrollBar {
                policy: ScrollBar.AsNeeded
                active: ScrollBar.AlwaysOn
            }

            ScrollBar.horizontal: ScrollBar {
                policy: ScrollBar.AsNeeded
                active: ScrollBar.AlwaysOn
            }
        }

        RowLayout {
            Layout.alignment: Qt.AlignHCenter
            spacing: 24
            Button {
                text: "← הקודם"
                enabled: currentIndex > 0
                onClicked: controller.setCurrentScheduleIndex(currentIndex - 1)
                background: Rectangle {
                    color: enabled ? "#1f2937" : "#9ca3af"
                    radius: 6
                }
                contentItem: Text {
                    text: parent.text
                    color: "#ffffff"
                    horizontalAlignment: Text.AlignHCenter
                    verticalAlignment: Text.AlignVCenter
                    font.pixelSize: 14
                }
                padding: 10
            }

            Button {
                text: "הבא →"
                enabled: currentIndex < totalSchedules - 1
                onClicked: controller.setCurrentScheduleIndex(currentIndex + 1)
                background: Rectangle {
                    color: enabled ? "#1f2937" : "#9ca3af"
                    radius: 6
                }
                contentItem: Text {
                    text: parent.text
                    color: "#ffffff"
                    horizontalAlignment: Text.AlignHCenter
                    verticalAlignment: Text.AlignVCenter
                    font.pixelSize: 14
                }
                padding: 10
            }
        }

        // --- Footer ---
        Rectangle {
            height: 60
            Layout.fillWidth: true
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
}