import QtQuick 2.15
import QtQuick.Controls 2.15

Popup {
    id: root

    // Only property we need - updated each time popup opens
    property int currentIndex: 0

    // Signals to be handled by parent
    signal printRequested()
    signal saveAsPngRequested()
    signal saveAsCsvRequested()

    width: 220
    height: menuColumn.height + 10
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
                text: "Export Schedule " + (root.currentIndex + 1)
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
                    root.printRequested()
                    root.close()
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
                    root.saveAsPngRequested()
                    root.close()
                }
            }
        }

        // Separator
        Rectangle {
            width: parent.width
            height: 1
            color: "#ffffff"
        }

        // Save as CSV option
        Rectangle {
            width: parent.width
            height: 50
            color: csvOptionArea.containsMouse ? "#415263" : "transparent"

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
                id: csvOptionArea
                anchors.fill: parent
                hoverEnabled: true
                cursorShape: Qt.PointingHandCursor
                onClicked: {
                    root.saveAsCsvRequested()
                    root.close()
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
                    root.close()
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