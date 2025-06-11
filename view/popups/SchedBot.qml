import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15
import QtQuick.Controls.Basic

Rectangle {
    id: chatBot
    width: 350
    height: parent.height
    color: "#ffffff"
    border.color: "#e5e7eb"
    border.width: 1

    // Expose isOpen property to parent
    property bool isOpen: false
    property alias messagesModel: messagesListModel

    // Reference to the controller for model operations
    property var controller: null

    // Hide completely when closed
    visible: isOpen

    // Animation for sliding in/out - only animate x position when visible
    x: parent.width - width

    Behavior on visible {
        NumberAnimation {
            duration: 300
            easing.type: Easing.OutCubic
        }
    }

    // Header
    Rectangle {
        id: header
        width: parent.width
        height: 60
        color: "#f8fafc"
        border.color: "#e2e8f0"
        border.width: 1

        RowLayout {
            anchors.fill: parent
            anchors.margins: 15
            spacing: 12

            // Bot icon
            Rectangle {
                Layout.preferredWidth: 32
                Layout.preferredHeight: 32
                color: "#3b82f6"
                radius: 16

                Text {
                    anchors.centerIn: parent
                    text: "🤖"
                    font.pixelSize: 16
                }
            }

            // Title
            Text {
                Layout.fillWidth: true
                text: "SchedBot"
                font.pixelSize: 18
                font.bold: true
                color: "#1f2937"
                verticalAlignment: Text.AlignVCenter
            }

            // Close button
            Button {
                Layout.preferredWidth: 32
                Layout.preferredHeight: 32

                background: Rectangle {
                    color: closeMouseArea.containsMouse ? "#f3f4f6" : "transparent"
                    radius: 6
                }

                contentItem: Text {
                    text: "×"
                    font.pixelSize: 18
                    color: "#6b7280"
                    horizontalAlignment: Text.AlignHCenter
                    verticalAlignment: Text.AlignVCenter
                }

                MouseArea {
                    id: closeMouseArea
                    anchors.fill: parent
                    hoverEnabled: true
                    cursorShape: Qt.PointingHandCursor
                    onClicked: chatBot.isOpen = false
                }
            }
        }
    }

    // Messages area
    Rectangle {
        id: messagesArea
        anchors {
            top: header.bottom
            left: parent.left
            right: parent.right
            bottom: inputArea.top
        }
        color: "#ffffff"

        ScrollView {
            id: scrollView
            anchors.fill: parent
            anchors.margins: 1
            clip: true

            ListView {
                id: messagesListView
                model: ListModel {
                    id: messagesListModel

                    Component.onCompleted: {
                        // Add initial bot message
                        append({
                            "isBot": true,
                            "message": "Hello! I'm here to help you with your schedule. I can help you find the perfect course conflicts, time preferences, or any scheduling questions.",
                            "timestamp": new Date().toLocaleTimeString(Qt.locale(), "hh:mm")
                        })
                    }
                }

                spacing: 12

                delegate: Item {
                    width: messagesListView.width
                    height: messageContainer.height + 12

                    Rectangle {
                        id: messageContainer
                        width: Math.min(parent.width * 0.8, messageText.implicitWidth + 24)
                        height: messageText.implicitHeight + 40 // Increased for timestamp space

                        anchors {
                            left: model.isBot ? parent.left : undefined
                            right: model.isBot ? undefined : parent.right
                            leftMargin: model.isBot ? 12 : 0
                            rightMargin: model.isBot ? 0 : 12
                        }

                        color: model.isBot ? "#f1f5f9" : "#3b82f6"
                        radius: 12
                        border.color: model.isBot ? "#e2e8f0" : "transparent"
                        border.width: 1

                        Text {
                            id: messageText
                            anchors {
                                left: parent.left
                                right: parent.right
                                top: parent.top
                                bottom: parent.bottom
                                margins: 12
                                bottomMargin: 24 // Extra space for timestamp
                            }

                            text: model.message
                            font.pixelSize: 14
                            color: model.isBot ? "#1f2937" : "#ffffff"
                            wrapMode: Text.WordWrap
                            verticalAlignment: Text.AlignTop
                        }

                        Text {
                            anchors {
                                right: parent.right
                                bottom: parent.bottom
                                margins: 8
                            }

                            text: model.timestamp
                            font.pixelSize: 10
                            color: model.isBot ? "#6b7280" : "#e0e7ff"
                        }
                    }
                }

                // Auto-scroll to bottom when new message is added
                onCountChanged: {
                    Qt.callLater(function() {
                        if (count > 0) {
                            positionViewAtEnd()
                        }
                    })
                }
            }
        }
    }

    // Input area
    Rectangle {
        id: inputArea
        anchors {
            left: parent.left
            right: parent.right
            bottom: parent.bottom
        }
        height: 70
        color: "#f8fafc"
        border.color: "#e2e8f0"
        border.width: 1

        RowLayout {
            anchors.fill: parent
            anchors.margins: 12
            spacing: 8

            ScrollView {
                Layout.fillWidth: true
                Layout.preferredHeight: Math.min(inputField.implicitHeight + 16, 100)

                TextArea {
                    id: inputField
                    placeholderText: "Ask about your schedule..."
                    placeholderTextColor: "#9ca3af"
                    font.pixelSize: 14
                    color: "#1f2937"
                    wrapMode: TextArea.Wrap
                    selectByMouse: true

                    background: Rectangle {
                        color: "#ffffff"
                        border.color: inputField.activeFocus ? "#3b82f6" : "#d1d5db"
                        border.width: 1
                        radius: 8
                    }

                    Keys.onReturnPressed: {
                        if (event.modifiers & Qt.ControlModifier) {
                            // Ctrl+Enter for new line
                            event.accepted = false
                        } else {
                            // Enter to send
                            sendMessage()
                            event.accepted = true
                        }
                    }
                }
            }

            Button {
                Layout.preferredWidth: 40
                Layout.preferredHeight: 40

                enabled: inputField.text.trim().length > 0

                background: Rectangle {
                    color: parent.enabled ? (sendMouseArea.containsMouse ? "#2563eb" : "#3b82f6") : "#e5e7eb"
                    radius: 8
                }

                contentItem: Text {
                    text: "→"
                    font.pixelSize: 16
                    font.bold: true
                    color: parent.enabled ? "#ffffff" : "#9ca3af"
                    horizontalAlignment: Text.AlignHCenter
                    verticalAlignment: Text.AlignVCenter
                }

                MouseArea {
                    id: sendMouseArea
                    anchors.fill: parent
                    hoverEnabled: true
                    cursorShape: parent.enabled ? Qt.PointingHandCursor : Qt.ForbiddenCursor
                    onClicked: {
                        if (parent.enabled) {
                            sendMessage()
                        }
                    }
                }
            }
        }
    }

    function sendMessage() {
        var messageText = inputField.text.trim()
        if (messageText.length === 0) return

        // Add user message
        messagesModel.append({
            "isBot": false,
            "message": messageText,
            "timestamp": new Date().toLocaleTimeString(Qt.locale(), "hh:mm")
        })

        // Clear input field
        inputField.text = ""

        // Send message to model through controller
        if (controller) {
            controller.processBotMessage(messageText)
        } else {
            // Fallback if controller is not available
            addBotResponse("I'm sorry, but I'm unable to process your request right now. Please try again later.")
        }
    }

    // Function to add bot response (called from controller)
    function addBotResponse(responseText) {
        messagesModel.append({
            "isBot": true,
            "message": responseText,
            "timestamp": new Date().toLocaleTimeString(Qt.locale(), "hh:mm")
        })
    }

    // Function to show typing indicator (optional enhancement)
    function showTypingIndicator() {
        messagesModel.append({
            "isBot": true,
            "message": "SchedBot is typing...",
            "timestamp": new Date().toLocaleTimeString(Qt.locale(), "hh:mm"),
            "isTyping": true
        })
    }

    // Function to remove typing indicator
    function removeTypingIndicator() {
        for (var i = messagesModel.count - 1; i >= 0; i--) {
            var item = messagesModel.get(i)
            if (item.isTyping) {
                messagesModel.remove(i)
                break
            }
        }
    }
}