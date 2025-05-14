// loading_overlay.qml
import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15
import QtQuick.Particles 2.15

Rectangle {
    id: root
    anchors.fill: parent
    color: Qt.rgba(0.12, 0.16, 0.23, 0.9)  // Semi-transparent dark blue-gray

    // Signal to connect to C++
    signal abortRequested()

    // Block mouse events so they don't reach underlying UI
    MouseArea {
        anchors.fill: parent
        // Prevent clicks from going through
        onClicked: {}
        onDoubleClicked: {}
        onPressAndHold: {}
    }

    // Reuse the particle system from your existing QML
    ParticleSystem {
        id: particleSystem
        anchors.fill: parent
        running: true

        ItemParticle {
            delegate: Rectangle {
                width: Math.random() * root.width * 0.01 + 3
                height: width
                radius: width/2
                color: Qt.rgba(0.7 + Math.random() * 0.3,
                    0.7 + Math.random() * 0.3,
                    0.7 + Math.random() * 0.3,
                    0.6 + Math.random() * 0.4)
            }
        }

        Emitter {
            anchors.horizontalCenter: parent.horizontalCenter
            anchors.bottom: parent.bottom
            width: parent.width * 0.9
            height: 10
            enabled: true
            lifeSpan: 8000
            emitRate: Math.max(10, root.width / 50) // Fixed the 'a' variable issue
            size: Math.max(5, root.width * 0.01)
            sizeVariation: size * 0.8
            velocity: AngleDirection {
                angle: 270
                angleVariation: 25
                magnitude: root.height * 0.07
                magnitudeVariation: magnitude * 0.7
            }
        }
    }

    // Dynamic bubbles (from your existing QML)
    Repeater {
        model: Math.min(24, Math.max(8, root.width / 80))

        Rectangle {
            id: bubble

            property real startX: Math.random() * root.width
            property real startSize: Math.random() * root.width * 0.04 + root.width * 0.02
            property real animDuration: Math.random() * 8000 + 4000
            property color bubbleColor: Qt.hsla(Math.random() * 0.2 + 0.5, 0.7, 0.7, 0.4)

            x: startX
            y: root.height + startSize
            width: startSize
            height: startSize
            radius: width/2
            color: bubbleColor
            opacity: 0.7

            SequentialAnimation {
                running: true
                loops: Animation.Infinite

                PauseAnimation { duration: index * 300 }

                ParallelAnimation {
                    NumberAnimation {
                        target: bubble
                        property: "y"
                        to: -bubble.startSize
                        duration: bubble.animDuration
                        easing.type: Easing.OutQuad
                    }
                    NumberAnimation {
                        target: bubble
                        property: "x"
                        to: bubble.startX + (Math.random() * root.width * 0.2 - root.width * 0.1)
                        duration: bubble.animDuration
                        easing.type: Easing.InOutSine
                    }
                    NumberAnimation {
                        target: bubble
                        property: "scale"
                        from: 1.0
                        to: Math.random() * 0.5 + 0.5
                        duration: bubble.animDuration
                        easing.type: Easing.InOutQuad
                    }
                }

                PropertyAction {
                    target: bubble
                    property: "y"
                    value: root.height + bubble.startSize
                }
                PropertyAction {
                    target: bubble
                    property: "x"
                    value: Math.random() * root.width
                }
                PropertyAction {
                    target: bubble
                    property: "scale"
                    value: 1.0
                }
                PropertyAction {
                    target: bubble
                    property: "bubbleColor"
                    value: Qt.hsla(Math.random() * 0.2 + 0.5, 0.7, 0.7, 0.4)
                }
            }
        }
    }

    // Your existing loading animation
    Item {
        id: loadingAnimation
        anchors.centerIn: parent
        width: Math.min(root.width * 0.3, root.height * 0.3)
        height: width

        // ... (your existing animation code)
        Item {
            anchors.fill: parent

            Rectangle {
                anchors.centerIn: parent
                width: parent.width * 0.8
                height: parent.height * 0.8
                radius: width/2
                color: "#2d3748"
                border.color: "#4a5568"
                border.width: Math.max(1, parent.width * 0.01)
            }

            Repeater {
                model: 12

                Rectangle {
                    id: orbitDot
                    property real angle: (index * 30) * (Math.PI / 180)
                    property real orbitRadius: loadingAnimation.width * 0.35

                    x: loadingAnimation.width/2 + Math.cos(angle) * orbitRadius - width/2
                    y: loadingAnimation.height/2 + Math.sin(angle) * orbitRadius - height/2
                    width: loadingAnimation.width * 0.06
                    height: width
                    radius: width/2
                    color: Qt.hsla(index/12, 0.8, 0.7, 0.8)

                    SequentialAnimation {
                        running: true
                        loops: Animation.Infinite

                        PauseAnimation { duration: index * 100 }

                        NumberAnimation {
                            target: orbitDot
                            property: "opacity"
                            from: 0.2
                            to: 1
                            duration: 500
                            easing.type: Easing.InOutQuad
                        }

                        NumberAnimation {
                            target: orbitDot
                            property: "opacity"
                            from: 1
                            to: 0.2
                            duration: 500
                            easing.type: Easing.InOutQuad
                        }
                    }
                }
            }

            RotationAnimation {
                target: loadingAnimation
                property: "rotation"
                from: 0
                to: 360
                duration: 12000
                loops: Animation.Infinite
                running: true
                easing.type: Easing.Linear
            }
        }

        Rectangle {
            anchors.centerIn: parent
            width: parent.width * 0.25
            height: width
            radius: width/2
            color: "#60a5fa"

            SequentialAnimation on scale {
                running: true
                loops: Animation.Infinite

                NumberAnimation {
                    from: 0.8
                    to: 1.2
                    duration: 1000
                    easing.type: Easing.InOutQuad
                }

                NumberAnimation {
                    from: 1.2
                    to: 0.8
                    duration: 1000
                    easing.type: Easing.InOutQuad
                }
            }
        }
    }

    // Your existing text and status
    Column {
        anchors.horizontalCenter: parent.horizontalCenter
        anchors.top: loadingAnimation.bottom
        anchors.topMargin: root.height * 0.05
        spacing: root.height * 0.02

        Text {
            id: loadingText
            text: "Generating schedules"
            anchors.horizontalCenter: parent.horizontalCenter
            color: "#e2e8f0"
            font.pixelSize: Math.max(16, root.height * 0.024)
            font.weight: Font.Medium
        }

        Row {
            anchors.horizontalCenter: parent.horizontalCenter
            spacing: root.width * 0.008

            Repeater {
                model: 3

                Rectangle {
                    width: Math.max(6, root.width * 0.012)
                    height: width
                    radius: width/2
                    color: "#60a5fa"
                    opacity: 0.2

                    SequentialAnimation on opacity {
                        running: true
                        loops: Animation.Infinite
                        NumberAnimation {
                            to: 1.0
                            duration: 500
                            easing.type: Easing.InOutQuad
                        }
                        NumberAnimation {
                            to: 0.2
                            duration: 500
                            easing.type: Easing.InOutQuad
                        }
                        PauseAnimation { duration: index * 300 }
                    }
                }
            }
        }

        // Status text
        Text {
            id: statusText
            anchors.horizontalCenter: parent.horizontalCenter
            color: "#e2e8f0"
            font.pixelSize: Math.max(12, root.height * 0.016)
            opacity: 0.8
            text: "Finding optimal course combinations..."

            property var messages: [
                "Finding optimal course combinations...",
                "Avoiding time conflicts...",
                "Organizing your schedule...",
                "Optimizing course selection...",
                "Almost done..."
            ]

            Timer {
                interval: 2000
                running: true
                repeat: true
                onTriggered: {
                    let currentIndex = statusText.messages.indexOf(statusText.text);
                    let nextIndex = (currentIndex + 1) % statusText.messages.length;
                    statusText.text = statusText.messages[nextIndex];
                }
            }

            Behavior on text {
                SequentialAnimation {
                    NumberAnimation { target: statusText; property: "opacity"; to: 0; duration: 300 }
                    PropertyAction { target: statusText; property: "text" }
                    NumberAnimation { target: statusText; property: "opacity"; to: 0.8; duration: 300 }
                }
            }
        }
    }
}