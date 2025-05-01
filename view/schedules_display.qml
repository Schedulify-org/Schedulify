import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15

Page {
    id: schedulesDisplayPage

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
                id: backButton
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

                onClicked: {
                    onClicked: schedulesDisplayController.goBack()
                }
            }

            // Screen Title
            Label {
                id: titleLabel
                text: "Generated schedules"
                font.pixelSize: 20
                color: "#1f2937"
                anchors.verticalCenter: parent.verticalCenter
            }
        }
    }
}
