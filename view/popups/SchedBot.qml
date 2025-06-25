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
    property bool isProcessing: false

    // Reference to the controller for model operations
    property var controller: null

    // Loading animation properties
    property int currentLoadingMessageIndex: 0
    property var loadingMessages: [
        "Scanning schedules now...",
        "Searching for the best result...",
        "Thinking about your question...",
        "Analyzing course patterns...",
        "Processing your request...",
        "Finding optimal matches...",
        "Reviewing schedule conflicts...",
        "Almost done..."
    ]

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

    // Loading message timer
    Timer {
        id: loadingMessageTimer
        interval: 1500  // Change message every 1.5 seconds
        repeat: true
        running: false
        onTriggered: {
            currentLoadingMessageIndex = (currentLoadingMessageIndex + 1) % loadingMessages.length
            updateLoadingMessage()
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

            // Bot icon with pulse animation when processing
            Rectangle {
                id: botIconRect
                Layout.preferredWidth: 32
                Layout.preferredHeight: 32
                color: "#3b82f6"
                radius: 16

                // Pulse animation when processing
                SequentialAnimation {
                    loops: Animation.Infinite
                    running: isProcessing

                    ScaleAnimator {
                        target: botIconRect
                        from: 1.0
                        to: 1.1
                        duration: 800
                        easing.type: Easing.InOutQuad
                    }
                    ScaleAnimator {
                        target: botIconRect
                        from: 1.1
                        to: 1.0
                        duration: 800
                        easing.type: Easing.InOutQuad
                    }
                }

                Text {
                    anchors.centerIn: parent
                    text: "🤖"
                    font.pixelSize: 16
                }
            }

            // Title with status indicator
            Column {
                Layout.fillWidth: true
                spacing: 2

                Text {
                    text: "SchedBot"
                    font.pixelSize: 18
                    font.bold: true
                    color: "#1f2937"
                    verticalAlignment: Text.AlignVCenter
                }

                Text {
                    id: statusText
                    text: isProcessing ? "Thinking..." : "Online"
                    font.pixelSize: 10
                    color: isProcessing ? "#f59e0b" : "#10b981"
                    visible: true

                    // Fade animation for status
                    OpacityAnimator {
                        target: statusText
                        from: 0.5
                        to: 1.0
                        duration: 1000
                        loops: Animation.Infinite
                        running: isProcessing
                    }
                }
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
                            "timestamp": new Date().toLocaleTimeString(Qt.locale(), "hh:mm"),
                            "isTyping": false
                        })
                    }
                }

                spacing: 16

                delegate: Item {
                    width: messagesListView.width
                    height: messageContainer.height + 16

                    Rectangle {
                        id: messageContainer
                        width: Math.min(parent.width * 0.85, messageText.implicitWidth + 24)
                        height: messageText.implicitHeight + 40

                        anchors {
                            left: model.isBot ? parent.left : undefined
                            right: model.isBot ? undefined : parent.right
                            leftMargin: model.isBot ? 16 : 0
                            rightMargin: model.isBot ? 0 : 16
                        }

                        color: model.isBot ? "#f8fafc" : "#3b82f6"
                        radius: 16
                        border.color: model.isBot ? "#e2e8f0" : "transparent"
                        border.width: 1

                        // Add subtle shadow effect
                        Rectangle {
                            anchors.fill: parent
                            anchors.topMargin: 2
                            color: model.isBot ? "#00000008" : "#00000015"
                            radius: parent.radius
                            z: -1
                        }

                        // Simple loading text - no animations
                        Text {
                            id: loadingText
                            anchors {
                                left: parent.left
                                right: parent.right
                                top: parent.top
                                bottom: parent.bottom
                                margins: 16
                                bottomMargin: 28
                            }

                            text: model.message
                            font.pixelSize: 14
                            color: "#6b7280"
                            wrapMode: Text.WordWrap
                            verticalAlignment: Text.AlignTop
                            visible: model.isTyping === true
                            font.italic: true
                        }

                        Text {
                            id: messageText
                            anchors {
                                left: parent.left
                                right: parent.right
                                top: parent.top
                                bottom: parent.bottom
                                margins: 16
                                bottomMargin: 28
                            }

                            text: model.message
                            font.pixelSize: 14
                            color: model.isBot ? "#1f2937" : "#ffffff"
                            wrapMode: Text.WordWrap
                            verticalAlignment: Text.AlignTop
                            visible: model.isTyping !== true

                            // Slide in animation for new messages
                            opacity: 0
                            Component.onCompleted: {
                                slideInAnimation.start()
                            }

                            OpacityAnimator {
                                id: slideInAnimation
                                target: messageText
                                from: 0
                                to: 1
                                duration: 300
                                easing.type: Easing.OutQuad
                            }
                        }

                        Text {
                            anchors {
                                right: parent.right
                                bottom: parent.bottom
                                margins: 10
                            }

                            text: model.timestamp
                            font.pixelSize: 10
                            color: model.isBot ? "#6b7280" : "#e0e7ff"
                            visible: model.isTyping !== true
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
                            event.accepted = false
                        } else {
                            sendMessage()
                            event.accepted = true
                        }
                    }
                }
            }

            Button {
                Layout.preferredWidth: 40
                Layout.preferredHeight: 40

                enabled: inputField.text.trim().length > 0 && !isProcessing

                background: Rectangle {
                    color: {
                        if (!parent.enabled) return "#e5e7eb"
                        if (isProcessing) return "#f59e0b"
                        return sendMouseArea.containsMouse ? "#2563eb" : "#3b82f6"
                    }
                    radius: 8
                }

                contentItem: Item {
                    anchors.fill: parent

                    // Regular send arrow
                    Text {
                        anchors.centerIn: parent
                        text: "→"
                        font.pixelSize: 16
                        font.bold: true
                        color: parent.parent.enabled ? "#ffffff" : "#9ca3af"
                        horizontalAlignment: Text.AlignHCenter
                        verticalAlignment: Text.AlignVCenter
                        visible: !isProcessing
                    }

                    // Enhanced loading spinner
                    Rectangle {
                        id: loadingSpinner
                        anchors.centerIn: parent
                        width: 20
                        height: 20
                        radius: 10
                        color: "transparent"
                        border.color: "#ffffff"
                        border.width: 2
                        visible: isProcessing

                        Rectangle {
                            anchors.top: parent.top
                            anchors.horizontalCenter: parent.horizontalCenter
                            width: 6
                            height: 6
                            radius: 3
                            color: "#ffffff"
                        }

                        RotationAnimation {
                            target: loadingSpinner
                            property: "rotation"
                            from: 0
                            to: 360
                            duration: 1000
                            loops: Animation.Infinite
                            running: isProcessing
                        }
                    }
                }

                MouseArea {
                    id: sendMouseArea
                    anchors.fill: parent
                    hoverEnabled: true
                    cursorShape: parent.enabled ? Qt.PointingHandCursor : Qt.ForbiddenCursor
                    onClicked: {
                        if (parent.enabled && !isProcessing) {
                            sendMessage()
                        }
                    }
                }
            }
        }
    }

    // Connect to controller's bot response signal
    Connections {
        target: controller
        function onBotResponseReceived(response) {
            addBotResponse(response)
        }
    }

    function sendMessage() {
        var messageText = inputField.text.trim()
        if (messageText.length === 0 || isProcessing) return

        // Add user message immediately
        messagesModel.append({
            "isBot": false,
            "message": messageText,
            "timestamp": new Date().toLocaleTimeString(Qt.locale(), "hh:mm"),
            "isTyping": false
        })

        // Clear input field
        inputField.text = ""

        // Set processing state
        isProcessing = true

        // Start animated loading messages
        currentLoadingMessageIndex = 0
        showAnimatedTypingIndicator()

        // Send message to model through controller
        if (controller) {
            controller.processBotMessage(messageText)
        } else {
            // Fallback if controller is not available
            hideTypingIndicator()
            addBotResponse("I'm sorry, but I'm unable to process your request right now. Please try again later.")
            isProcessing = false
        }
    }

    function addBotResponse(responseText) {
        hideTypingIndicator()
        messagesModel.append({
            "isBot": true,
            "message": responseText,
            "timestamp": new Date().toLocaleTimeString(Qt.locale(), "hh:mm"),
            "isTyping": false
        })
        isProcessing = false
    }

    function showAnimatedTypingIndicator() {
        messagesModel.append({
            "isBot": true,
            "message": loadingMessages[currentLoadingMessageIndex],
            "timestamp": new Date().toLocaleTimeString(Qt.locale(), "hh:mm"),
            "isTyping": true
        })

        // Start the timer to cycle through loading messages
        loadingMessageTimer.start()
    }

    function updateLoadingMessage() {
        // Find and update the typing indicator message
        for (var i = messagesModel.count - 1; i >= 0; i--) {
            var item = messagesModel.get(i)
            if (item.isTyping) {
                messagesModel.setProperty(i, "message", loadingMessages[currentLoadingMessageIndex])
                break
            }
        }
    }

    function hideTypingIndicator() {
        loadingMessageTimer.stop()

        for (var i = messagesModel.count - 1; i >= 0; i--) {
            var item = messagesModel.get(i)
            if (item.isTyping) {
                messagesModel.remove(i)
                break
            }
        }
    }
}