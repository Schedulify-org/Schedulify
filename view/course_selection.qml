// course_selection.qml
import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15

Page {
    id: courseSelectionPage

    header: ToolBar {
        RowLayout {
            anchors.fill: parent

            Button {
                text: "Back"
                onClicked: courseSelectionController.goBack()
            }

            Label {
                text: "Schedule Builder - Course Selection"
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
            text: "Select Courses"
            font.pixelSize: 18
            font.bold: true
        }

        ScrollView {
            Layout.fillWidth: true
            Layout.fillHeight: true

            ListView {
                id: coursesListView
                anchors.fill: parent
                model: courseSelectionController.courseModel
                delegate: ItemDelegate {
                    width: parent.width
                    height: 60

                    RowLayout {
                        anchors.fill: parent
                        anchors.margins: 10
                        spacing: 10

                        CheckBox {
                            id: courseCheckBox
                        }

                        ColumnLayout {
                            Layout.fillWidth: true
                            spacing: 5

                            Label {
                                text: courseId + ": " + courseName
                                font.pixelSize: 16
                                Layout.fillWidth: true
                            }

                            Label {
                                text: "Instructor: " + teacherName
                                font.pixelSize: 14
                                color: "gray"
                                Layout.fillWidth: true
                            }
                        }
                    }
                }
            }
        }

        Button {
            text: "Generate Schedules"
            Layout.alignment: Qt.AlignHCenter
            Layout.preferredWidth: 200
            Layout.preferredHeight: 50
            font.pixelSize: 16
            onClicked: courseSelectionController.generateSchedules()
        }
    }
}