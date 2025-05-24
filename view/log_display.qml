import QtQuick 2.15
import QtQuick.Window 2.15
import QtQuick.Layouts 1.15
import QtQuick.Controls 2.15
import QtQuick.Controls.Basic

Window {
    id: logWindow
    width: 700
    height: 500
    title: "Schedulify Logs"
    color: "#f9fafb"

    flags: Qt.Window

    onVisibleChanged: {
        if (visible && logDisplayController) {
            logDisplayController.refreshLogs();
        }
    }

    Connections {
        target: logDisplayController

        function onLogEntriesChanged() {
            // Force the ListView to update by reassigning the model
            logListView.model = null;
            logListView.model = logDisplayController.logEntries;
        }
    }

    Item {
        focus: true
        Keys.onEscapePressed: logWindow.close()
    }

    Rectangle {
        anchors.fill: parent
        color: "#f9fafb"

        // Header with title
        Rectangle {
            id: logHeader
            width: parent.width
            height: 60
            color: "#ffffff"
            border.color: "#e5e7eb"

            Text {
                id: headerText
                anchors {
                    left: parent.left
                    verticalCenter: parent.verticalCenter
                    leftMargin: 20
                }
                text: "Schedulify Logs"
                font.pixelSize: 18
                font.bold: true
                color: "#1f2937"
            }

            // Refresh button
            Button {
                anchors {
                    right: parent.right
                    verticalCenter: parent.verticalCenter
                    rightMargin: 10
                }
                width: 32
                height: 32
                Text {
                    text: "🔄"
                    anchors.centerIn: parent
                    font.pixelSize: 16
                    color: "#1f2937"
                }

                background: Rectangle {
                    color: "#ffffff"
                    radius: 16
                }

                MouseArea {
                    id: refreshMouseArea
                    anchors.fill: parent
                    hoverEnabled: true
                    cursorShape: Qt.PointingHandCursor
                    onClicked: {
                        if (logDisplayController) {
                            logDisplayController.refreshLogs();
                        }
                    }
                }

                ToolTip {
                    visible: refreshMouseArea.containsMouse
                    text: "Refresh logs"
                    delay: 500
                }
            }
        }

        // Logs ListView
        ListView {
            id: logListView
            anchors {
                top: logHeader.bottom
                left: parent.left
                right: parent.right
                bottom: parent.bottom
                margins: 1
            }
            clip: true

            // Set both model binding and initial value
            model: logDisplayController ? logDisplayController.logEntries : []
            spacing: 1

            // Auto-scroll to bottom when content changes
            onCountChanged: {
                if (count > 0) {
                    positionViewAtEnd();
                }
            }

            ScrollBar.vertical: ScrollBar {
                active: true

                // Keep it at bottom when new items are added
                onPositionChanged: {
                    if (position + size < 1.0) {
                        // User has manually scrolled up, don't auto-scroll
                        logListView.autoScroll = false;
                    } else {
                        // At bottom, enable auto-scroll
                        logListView.autoScroll = true;
                    }
                }
            }

            property bool autoScroll: true

            // The rest of your delegate code remains the same
            delegate: Rectangle {
                width: logListView.width
                height: contentLayout.implicitHeight + 16
                color: "#ffffff"

                // Separator line
                Rectangle {
                    anchors.left: parent.left
                    anchors.right: parent.right
                    anchors.bottom: parent.bottom
                    height: 1
                    color: "#e5e7eb"
                }

                RowLayout {
                    id: contentLayout
                    anchors.fill: parent
                    anchors.margins: 12
                    spacing: 12

                    // Log level indicator
                    Rectangle {
                        Layout.preferredWidth: 6
                        Layout.fillHeight: true
                        color: modelData.color
                        radius: 3
                    }

                    // Log level badge
                    Rectangle {
                        Layout.preferredWidth: 80
                        Layout.preferredHeight: 26
                        color: Qt.alpha(modelData.color, 0.15)
                        radius: 4

                        Text {
                            anchors.centerIn: parent
                            text: modelData.level
                            color: modelData.color
                            font.pixelSize: 12
                            font.bold: true
                        }
                    }

                    // Timestamp
                    Text {
                        Layout.preferredWidth: 140
                        text: modelData.timestamp
                        color: "#64748b"
                        font.pixelSize: 12
                        font.family: "Consolas, monospace"
                    }

                    // Message
                    Text {
                        Layout.fillWidth: true
                        text: modelData.message
                        color: "#1e293b"
                        font.pixelSize: 14
                        wrapMode: Text.Wrap
                    }
                }
            }

            // Empty state
            Rectangle {
                anchors.centerIn: parent
                width: 300
                height: 100
                color: "transparent"
                visible: logDisplayController ? logDisplayController.logEntries.length === 0 : true

                Column {
                    anchors.centerIn: parent
                    spacing: 10

                    Text {
                        anchors.horizontalCenter: parent.horizontalCenter
                        text: "📋"
                        font.pixelSize: 32
                        color: "#9ca3af"
                    }

                    Text {
                        text: "No logs available"
                        font.pixelSize: 16
                        color: "#6b7280"
                    }
                }
            }
        }
    }

    Component.onDestruction: {
        if (logDisplayController) {
            logDisplayController.setLogWindowOpen(false);
        }
    }
}