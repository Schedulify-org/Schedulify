// schedules_display.qml
import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15

Page {
    id: schedulesDisplayPage

    header: ToolBar {
        RowLayout {
            anchors.fill: parent

            Button {
                text: "Back"
                onClicked: schedulesDisplayController.goBack()
            }

            Label {
                text: "Schedule Builder - Generated Schedules"
                font.pixelSize: 20
                elide: Label.ElideRight
                horizontalAlignment: Qt.AlignHCenter
                verticalAlignment: Qt.AlignVCenter
                Layout.fillWidth: true
            }
        }
    }

    ColumnLayout {
        anchors.fill: parent
        anchors.margins: 20
        spacing: 20

        Label {
            text: "Generated Schedules"
            font.pixelSize: 18
            font.bold: true
        }

        TabBar {
            id: scheduleTabBar
            Layout.fillWidth: true

            TabButton {
                text: "Schedule 1"
            }

            TabButton {
                text: "Schedule 2"
            }

            TabButton {
                text: "Schedule 3"
            }
        }

        StackLayout {
            Layout.fillWidth: true
            Layout.fillHeight: true
            currentIndex: scheduleTabBar.currentIndex

            // Schedule 1
            Item {
                Rectangle {
                    anchors.fill: parent
                    color: "white"
                    border.color: "lightgray"
                    border.width: 1

                    ScrollView {
                        anchors.fill: parent
                        anchors.margins: 10

                        Column {
                            width: parent.width
                            spacing: 10

                            // Sample schedule layout - would be generated dynamically
                            Repeater {
                                model: ["Monday", "Tuesday", "Wednesday", "Thursday", "Friday"]

                                Column {
                                    width: parent.width
                                    spacing: 5

                                    Label {
                                        text: modelData
                                        font.bold: true
                                        font.pixelSize: 16
                                    }

                                    Rectangle {
                                        width: parent.width
                                        height: 1
                                        color: "lightgray"
                                    }

                                    // Sample courses for this day
                                    Repeater {
                                        model: 2

                                        Rectangle {
                                            width: parent.width
                                            height: 50
                                            color: "aliceblue"
                                            border.color: "lightblue"
                                            border.width: 1
                                            radius: 5

                                            Label {
                                                anchors.centerIn: parent
                                                text: "Sample Course " + (index + 1)
                                            }
                                        }
                                    }
                                }
                            }
                        }
                    }
                }
            }

            // Schedule 2
            Item {
                Rectangle {
                    anchors.fill: parent
                    color: "white"
                    border.color: "lightgray"
                    border.width: 1

                    Label {
                        anchors.centerIn: parent
                        text: "Schedule 2 Content"
                    }
                }
            }

            // Schedule 3
            Item {
                Rectangle {
                    anchors.fill: parent
                    color: "white"
                    border.color: "lightgray"
                    border.width: 1

                    Label {
                        anchors.centerIn: parent
                        text: "Schedule 3 Content"
                    }
                }
            }
        }

        RowLayout {
            Layout.alignment: Qt.AlignHCenter
            spacing: 20

            Button {
                text: "Save Schedule"
                Layout.preferredWidth: 150
                onClicked: schedulesDisplayController.saveSchedule(scheduleTabBar.currentIndex)
            }

            Button {
                text: "Print Schedule"
                Layout.preferredWidth: 150
                onClicked: schedulesDisplayController.printSchedule(scheduleTabBar.currentIndex)
            }
        }
    }
}