import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15
import Qt.labs.qmlmodels 1.0

Page {
    id: schedulesDisplayPage
    background: Rectangle {
        color: "#ffffff"
    }
    property var controller: schedulesDisplayController
    property int currentIndex: controller.currentScheduleIndex
    property int totalSchedules: controller.getScheduleCount()

    ColumnLayout {
        anchors.fill: parent
        spacing: 16
        anchors.margins: 24

        Label {
            text: "מערכת נוכחית: " + (currentIndex + 1) + " / " + totalSchedules
            font.pixelSize: 24
            color: "#3a3e45"
        }

        Rectangle {
            Layout.fillWidth: true
            Layout.fillHeight: true
            radius: 8
            border.color: "#d1d5db"
            color: "white"

            ColumnLayout {
                anchors.fill: parent
                spacing: 0

                // כותרות ימים
                Rectangle {
                    Layout.fillWidth: true
                    height: 40
                    color: "#f3f4f6"

                    Row {
                        anchors.fill: parent

                        Rectangle {
                            width: 70
                            height: parent.height
                            color: "#e5e7eb"
                            border.width: 1
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
                                width: 150
                                height: parent.height
                                color: "#e5e7eb"
                                border.width: 1
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
                }

                // טבלת המערכת
                TableView {
                    id: scheduleTable
                    Layout.fillWidth: true
                    Layout.fillHeight: true
                    clip: true
                    rowSpacing: 1
                    columnSpacing: 1

                    property var timeSlots: [
                        "8:00-9:00", "9:00-10:00", "10:00-11:00", "11:00-12:00",
                        "12:00-13:00", "13:00-14:00", "14:00-15:00", "15:00-16:00", "16:00-17:00",
                        "17:00-18:00", "18:00-19:00", "19:00-20:00"
                    ]

                    columnWidthProvider: function(col) {
                        return col === 0 ? 70 : 150;
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
                                for (let d = 0; d < 7; d++) {
                                    let dayName = days[d];
                                    let items = controller.getDayItems(currentIndex, d);

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
                                                        item.courseName + "\n" +
                                                        item.raw_id + " - " + item.type + "\n" +
                                                        item.start + " - " + item.end + "\n" +
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
                                ? "#94a0b0"
                                : "#ffffff")

                        Text {
                            anchors.fill: parent
                            wrapMode: Text.WordWrap
                            verticalAlignment: Text.AlignVCenter
                            horizontalAlignment: Text.AlignLeft
                            padding: 6
                            font.pixelSize: 11
                            text: model.display ? String(model.display) : ""
                            color: "#2e2e2e"
                        }
                    }
                }
            }
        }

        RowLayout {
            Layout.alignment: Qt.AlignHCenter
            spacing: 24

            // ← הקודם Button
            Rectangle {
                id: prevButton
                radius: 4
                color: prevMouseArea.containsMouse ? "#35455c" : "#1f2937"
                implicitWidth: 120
                implicitHeight: 40

                Layout.preferredWidth: 140
                visible: currentIndex > 0

                Text {
                    text: "← הקודם"
                    anchors.centerIn: parent
                    color: "white"
                    font.bold: true
                    horizontalAlignment: Text.AlignHCenter
                    verticalAlignment: Text.AlignVCenter
                }

                MouseArea {
                    id: prevMouseArea
                    anchors.fill: parent
                    hoverEnabled: true
                    enabled: currentIndex > 0
                    cursorShape: Qt.PointingHandCursor
                    onClicked: controller.setCurrentScheduleIndex(currentIndex - 1)
                }
            }

            // הבא → Button
            Rectangle {
                id: nextButton
                radius: 4
                color: nextMouseArea.containsMouse ? "#35455c" : "#1f2937"
                implicitWidth: 120
                implicitHeight: 40

                Layout.preferredWidth: 140
                visible: currentIndex < totalSchedules - 1

                Text {
                    text: "הבא →"
                    anchors.centerIn: parent
                    color: "white"
                    font.bold: true
                    horizontalAlignment: Text.AlignHCenter
                    verticalAlignment: Text.AlignVCenter
                }

                MouseArea {
                    id: nextMouseArea
                    anchors.fill: parent
                    hoverEnabled: true
                    enabled: currentIndex < totalSchedules - 1
                    cursorShape: Qt.PointingHandCursor
                    onClicked: controller.setCurrentScheduleIndex(currentIndex + 1)
                }
            }

            // Save PDF Button
            Rectangle {
                id: saveButtonRect
                radius: 4
                color: saveMouseArea.containsMouse ? "#35455c" : "#1f2937"
                implicitWidth: 140
                implicitHeight: 40

                Text {
                    text: "שמור כ-PDF"
                    anchors.centerIn: parent
                    color: "white"
                    font.bold: true
                    horizontalAlignment: Text.AlignHCenter
                    verticalAlignment: Text.AlignVCenter
                }

                MouseArea {
                    id: saveMouseArea
                    anchors.fill: parent
                    hoverEnabled: true
                    cursorShape: Qt.PointingHandCursor
                    onClicked: controller.saveScheduleAsPDF()
                }
            }

            // Print Button
            Rectangle {
                id: printButtonRect
                radius: 4
                color: printMouseArea.containsMouse ? "#35455c" : "#1f2937"
                implicitWidth: 140
                implicitHeight: 40

                Text {
                    text: "הדפסה"
                    anchors.centerIn: parent
                    color: "white"
                    font.bold: true
                    horizontalAlignment: Text.AlignHCenter
                    verticalAlignment: Text.AlignVCenter
                }

                MouseArea {
                    id: printMouseArea
                    anchors.fill: parent
                    hoverEnabled: true
                    cursorShape: Qt.PointingHandCursor
                    onClicked: controller.printScheduleDirectly()
                }
            }
        }
        }

}