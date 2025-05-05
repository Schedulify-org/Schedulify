import QtQuick 2.15
import QtQuick.Layouts 1.15
import Qt.labs.qmlmodels 1.0
import QtQuick.Controls 2.15
import QtQuick.Controls.Basic

Page {
    id: schedulesDisplayPage

    property var controller: schedulesDisplayController
    property int currentIndex: controller.currentScheduleIndex
    property int totalSchedules: controller.getScheduleCount()

    Rectangle {
        id: root
        anchors.fill: parent
        color: "#f9fafb"

        // Header
        Rectangle {
            id: header
            width: parent.width
            height: 80
            color: "#ffffff"
            border.color: "#e5e7eb"

            Row {
                anchors {
                    left: parent.left
                    verticalCenter: parent.verticalCenter
                    leftMargin: 16
                }
                spacing: 16

                // Back Button
                Button {
                    id: schedulesBackButton
                    width: 40
                    height: 40
                    background: Rectangle {
                        color: schedulesBackMouseArea.containsMouse ? "#a8a8a8" : "#f3f4f6"
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
                        id: schedulesBackMouseArea
                        anchors.fill: parent
                        hoverEnabled: true
                        onClicked: controller.goBack()
                        cursorShape: Qt.PointingHandCursor
                    }
                }

                // Screen Title
                Label {
                    id: titleLabel
                    text: "מערכות שעות שנוצרו"
                    font.pixelSize: 20
                    color: "#1f2937"
                    anchors {
                        left: schedulesBackButton.right
                        leftMargin: 16
                        verticalCenter: parent.verticalCenter
                    }
                }
            }
        }

        // Layout for content
        ColumnLayout {
            anchors {
                top: header.bottom
                left: parent.left
                right: parent.right
                bottom: footer.top
                margins: 24
            }
            spacing: 16

            // Schedule index indicator
            Label {
                text: "מערכת שעות " + (currentIndex + 1) + " מתוך " + totalSchedules
                font.pixelSize: 16
                color: "#4b5563"
            }

            // Schedule view area
            Rectangle {
                Layout.fillWidth: true
                Layout.fillHeight: true
                color: "#ffffff"
                radius: 8
                border.color: "#d1d5db"

                // כותרות עמודות
                Row {
                    id: headerRow
                    anchors {
                        top: parent.top
                        left: parent.left
                        right: parent.right
                        margins: 10
                    }
                    height: 30

                    Rectangle { width: 70; height: parent.height; color: "#f3f4f6"; border.color: "#e5e7eb"; border.width: 1; Text { anchors.centerIn: parent; text: "שעה"; font.bold: true; color: "#1f2937" } }
                    Rectangle { width: 150; height: parent.height; color: "#f3f4f6"; border.color: "#e5e7eb"; border.width: 1; Text { anchors.centerIn: parent; text: "ראשון"; font.bold: true; color: "#1f2937" } }
                    Rectangle { width: 150; height: parent.height; color: "#f3f4f6"; border.color: "#e5e7eb"; border.width: 1; Text { anchors.centerIn: parent; text: "שני"; font.bold: true; color: "#1f2937" } }
                    Rectangle { width: 150; height: parent.height; color: "#f3f4f6"; border.color: "#e5e7eb"; border.width: 1; Text { anchors.centerIn: parent; text: "שלישי"; font.bold: true; color: "#1f2937" } }
                    Rectangle { width: 150; height: parent.height; color: "#f3f4f6"; border.color: "#e5e7eb"; border.width: 1; Text { anchors.centerIn: parent; text: "רביעי"; font.bold: true; color: "#1f2937" } }
                    Rectangle { width: 150; height: parent.height; color: "#f3f4f6"; border.color: "#e5e7eb"; border.width: 1; Text { anchors.centerIn: parent; text: "חמישי"; font.bold: true; color: "#1f2937" } }
                    Rectangle { width: 150; height: parent.height; color: "#f3f4f6"; border.color: "#e5e7eb"; border.width: 1; Text { anchors.centerIn: parent; text: "שישי"; font.bold: true; color: "#1f2937" } }
                    Rectangle { width: 150; height: parent.height; color: "#f3f4f6"; border.color: "#e5e7eb"; border.width: 1; Text { anchors.centerIn: parent; text: "שבת"; font.bold: true; color: "#1f2937" } }
                }

                // Schedule Grid
                TableView {
                    id: scheduleTableView
                    anchors {
                        top: headerRow.bottom
                        left: parent.left
                        right: parent.right
                        bottom: parent.bottom
                        margins: 10
                        topMargin: 0
                    }
                    clip: true
                    rowSpacing: 1
                    columnSpacing: 1

                    property var timeSlots: [
                        "8:00", "9:00", "10:00", "11:00", "12:00",
                        "13:00", "14:00", "15:00", "16:00", "17:00", "18:00", "19:00", "20:00"
                    ]

                    // מגדיר את הרוחב של העמודות
                    columnWidthProvider: function(column) {
                        return column === 0 ? 70 : 150;
                    }

                    // כותרות עמודות מחוץ לטבלה (מכיוון ש-header אינו נתמך)
                    topMargin: 31 // שומר מקום לכותרות

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
                            let scheduleData = [];

                            for (let i = 0; i < scheduleTableView.timeSlots.length; i++) {
                                let row = {
                                    timeSlot: scheduleTableView.timeSlots[i],
                                    sunday: null,
                                    monday: null,
                                    tuesday: null,
                                    wednesday: null,
                                    thursday: null,
                                    friday: null,
                                    saturday: null
                                };
                                scheduleData.push(row);
                            }

                            if (totalSchedules > 0 && currentIndex >= 0) {
                                for (let day = 0; day < 7; day++) {
                                    let dayItems = controller.getDayItems(controller.currentScheduleIndex, day);
                                    let dayName = "";

                                    switch (day) {
                                        case 0: dayName = "sunday"; break;
                                        case 1: dayName = "monday"; break;
                                        case 2: dayName = "tuesday"; break;
                                        case 3: dayName = "wednesday"; break;
                                        case 4: dayName = "thursday"; break;
                                        case 5: dayName = "friday"; break;
                                        case 6: dayName = "saturday"; break; // הוסף את זה
                                    }

                                    for (let i = 0; i < dayItems.length; i++) {
                                        let item = dayItems[i];
                                        let startTime = item.start.split(':');
                                        let startHour = parseInt(startTime[0]);
                                        let startMinute = parseInt(startTime[1]);

                                        let rowIndex = scheduleTableView.timeSlots.findIndex(
                                                slot => {
                                                let slotHour = parseInt(slot.split(':')[0]);
                                                return slotHour === startHour;
                                            }
                                        );

                                        if (rowIndex >= 0) {
                                            scheduleData[rowIndex][dayName] = {
                                                courseName: item.courseName,
                                                rawId: item.raw_id,
                                                type: item.type,
                                                startTime: item.start,
                                                endTime: item.end,
                                                building: item.building,
                                                room: item.room
                                            };
                                        }
                                    }
                                }
                            }

                            return scheduleData;
                        }
                    }

                    delegate: Rectangle {
                        // רוחב מוגדר על ידי columnWidthProvider
                        implicitHeight: 80
                        border.width: 1
                        border.color: "#e5e7eb"
                        color: model.column === 0 ? "#f3f4f6" : "#ffffff"

                        // Time column
                        Text {
                            visible: model.column === 0
                            anchors.centerIn: parent
                            text: model.display
                            font.pixelSize: 14
                            color: "#4b5563"
                        }

                        // Course cells
                        Rectangle {
                            visible: model.column > 0 && model.display !== null
                            anchors.fill: parent
                            anchors.margins: 2
                            color: "#e7f2fd"
                            radius: 4

                            Column {
                                anchors.fill: parent
                                anchors.margins: 4
                                spacing: 2

                                Text {
                                    width: parent.width
                                    text: model.display ? model.display.courseName : ""
                                    font.pixelSize: 12
                                    font.bold: true
                                    elide: Text.ElideRight
                                    color: "#1f2937"
                                }

                                Text {
                                    width: parent.width
                                    text: model.display ? model.display.rawId + " - " + model.display.type : ""
                                    font.pixelSize: 11
                                    elide: Text.ElideRight
                                    color: "#4b5563"
                                }

                                Text {
                                    width: parent.width
                                    text: model.display ? model.display.startTime + " - " + model.display.endTime : ""
                                    font.pixelSize: 10
                                    color: "#6b7280"
                                }

                                Text {
                                    width: parent.width
                                    text: model.display ? "בניין: " + model.display.building + ", חדר: " + model.display.room : ""
                                    font.pixelSize: 10
                                    elide: Text.ElideRight
                                    color: "#6b7280"
                                }
                            }
                        }
                    }

                    Rectangle {
                        z: -1
                        anchors.fill: parent
                        color: "transparent"
                        border.color: "#d1d5db"
                    }
                }
            }

            // Navigation buttons row
            RowLayout {
                Layout.alignment: Qt.AlignHCenter
                spacing: 16

                Button {
                    id: prevButton
                    width: 180
                    height: 40
                    enabled: currentIndex > 0
                    background: Rectangle {
                        color: enabled ? (prevMouseArea.containsMouse ? "#35455c" : "#1f2937") : "#9ca3af"
                        radius: 4
                        implicitWidth: 180
                        implicitHeight: 40
                    }
                    font.bold: true
                    contentItem: Text {
                        text: "← הקודם"
                        color: "white"
                        horizontalAlignment: Text.AlignHCenter
                        verticalAlignment: Text.AlignVCenter
                    }
                    MouseArea {
                        id: prevMouseArea
                        anchors.fill: parent
                        hoverEnabled: true
                        enabled: parent.enabled
                        onClicked: controller.setCurrentScheduleIndex(currentIndex - 1)
                        cursorShape: enabled ? Qt.PointingHandCursor : Qt.ArrowCursor
                    }
                }

                Button {
                    id: nextButton
                    width: 180
                    height: 40
                    enabled: currentIndex < totalSchedules - 1
                    background: Rectangle {
                        color: enabled ? (forwardMouseArea.containsMouse ? "#35455c" : "#1f2937") : "#9ca3af"
                        radius: 4
                        implicitWidth: 180
                        implicitHeight: 40
                    }
                    font.bold: true
                    contentItem: Text {
                        text: "הבא →"
                        color: "white"
                        horizontalAlignment: Text.AlignHCenter
                        verticalAlignment: Text.AlignVCenter
                    }
                    MouseArea {
                        id: forwardMouseArea
                        anchors.fill: parent
                        hoverEnabled: true
                        enabled: parent.enabled
                        onClicked: controller.setCurrentScheduleIndex(currentIndex + 1)
                        cursorShape: enabled ? Qt.PointingHandCursor : Qt.ArrowCursor
                    }
                }
            }
        }

        // Footer
        Rectangle {
            id: footer
            anchors {
                bottom: parent.bottom
                left: parent.left
                right: parent.right
            }
            height: 60
            width: parent.width
            color: "#ffffff"
            border.color: "#e5e7eb"

            // Footer Text
            Label {
                anchors.centerIn: parent
                text: "© 2025 Schedulify. כל הזכויות שמורות."
                color: "#6b7280"
                font.pixelSize: 12
            }
        }
    }
}