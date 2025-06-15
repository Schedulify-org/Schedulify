import QtQuick 2.15
import QtQuick.Layouts 1.15
import QtQuick.Controls 2.15
import QtQuick.Controls.Basic
import "."

Page {
    id: inputScreen

    // State for toggling between new file and history
    property bool showHistory: false

    Connections {
        target: fileInputController

        function onInvalidFileFormat() {
            showErrorMessage("Invalid file format. Please upload a valid course list.");
        }

        function onErrorMessage(message) {
            showErrorMessage(message);
        }

        function onFileSelected(hasFile) {
            continueButton.visible = hasFile;
        }

        function onFileNameChanged(fileName) {
            fileNameText.text = fileName;
            fileNameText.visible = true;
            dropPrompt.visible = false;
        }

        function onFileSelectionChanged() {
            loadHistoryButton.enabled = fileInputController.selectedFileCount > 0;
            selectedCountText.text = fileInputController.selectedFileCount > 0 ?
                `${fileInputController.selectedFileCount} file(s) selected` : "";

            // Force update of list view to refresh checkbox states
            if (fileHistoryList.model) {
                fileHistoryList.model.forceRefresh();
            }
        }
    }

    property var logWindow: null

    Component.onDestruction: {
        if (logWindow) {
            logWindow.destroy();
            logWindow = null;
            if (logDisplayController) {
                logDisplayController.setLogWindowOpen(false);
            }
        }
    }

    // Add this function for error dialogs
    property string errorDialogText: ""

    Dialog {
        id: errorDialog
        title: "Error"
        standardButtons: Dialog.Ok

        Text {
            text: errorDialogText
            wrapMode: Text.WordWrap
        }
    }

    function showErrorMessage(msg) {
        errorDialogText = msg;
        errorDialog.open();
    }

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

            Label {
                id: titleLabel
                x: 16
                y: 28
                text: "Schedule Builder"
                font.pixelSize: 20
                color: "#1f2937"
            }

            Row {
                anchors {
                    right: parent.right
                    verticalCenter: parent.verticalCenter
                    rightMargin: 15
                }
                spacing: 10

                // Refresh button for file history
                Button {
                    id: refreshButton
                    width: 40
                    height: 40
                    visible: showHistory

                    background: Rectangle {
                        color: refreshMouseArea.containsMouse ? "#a8a8a8" : "#f3f4f6"
                        radius: 10
                    }

                    contentItem: Item {
                        anchors.fill: parent

                        Text {
                            anchors.centerIn: parent
                            text: "↻"
                            font.pixelSize: 18
                            color: "#374151"
                        }

                        ToolTip {
                            id: refreshTooltip
                            text: "Refresh File History"
                            visible: refreshMouseArea.containsMouse
                            delay: 500
                            timeout: 3000
                        }
                    }

                    MouseArea {
                        id: refreshMouseArea
                        anchors.fill: parent
                        hoverEnabled: true
                        cursorShape: Qt.PointingHandCursor
                        onClicked: {
                            console.log("Refreshing file history...");
                            fileInputController.refreshFileHistory();
                        }
                    }
                }

                // Log button
                Button {
                    id: logButtonA
                    width: 40
                    height: 40

                    background: Rectangle {
                        color: logMouseArea.containsMouse ? "#a8a8a8" : "#f3f4f6"
                        radius: 10
                    }

                    contentItem: Item {
                        anchors.fill: parent

                        Image {
                            id: logIcon
                            anchors.centerIn: parent
                            width: 24
                            height: 24
                            source: "qrc:/icons/ic-logs.svg"
                            sourceSize.width: 22
                            sourceSize.height: 22
                        }

                        ToolTip {
                            id: logsTooltip
                            text: "Open Application Logs"
                            visible: logMouseArea.containsMouse
                            delay: 500
                            timeout: 3000

                            background: Rectangle {
                                color: "#374151"
                                radius: 4
                                border.color: "#4b5563"
                            }

                            contentItem: Text {
                                text: logsTooltip.text
                                color: "white"
                                font.pixelSize: 12
                            }
                        }
                    }

                    MouseArea {
                        id: logMouseArea
                        anchors.fill: parent
                        hoverEnabled: true
                        cursorShape: Qt.PointingHandCursor
                        onClicked: {
                            if (logWindow && logWindow.visible) {
                                logWindow.raise();
                                logWindow.requestActivate();
                                return;
                            }

                            if (!logDisplayController.isLogWindowOpen || !logWindow) {
                                var component = Qt.createComponent("qrc:/log_display.qml");
                                if (component.status === Component.Ready) {
                                    logDisplayController.setLogWindowOpen(true);
                                    logWindow = component.createObject(inputScreen);

                                    if (logWindow) {
                                        logWindow.closing.connect(function(close) {
                                            logDisplayController.setLogWindowOpen(false);
                                            logWindow = null;
                                        });

                                        logWindow.show();
                                    }
                                } else {
                                    console.error("Error creating log window:", component.errorString());
                                }
                            }
                        }
                    }
                }
            }
        }

        // Toggle Button Section
        Rectangle {
            id: toggleSection
            anchors {
                top: header.bottom
                left: parent.left
                right: parent.right
                topMargin: 20
            }
            height: 60
            color: "transparent"

            Rectangle {
                id: toggleContainer
                anchors.centerIn: parent
                width: 300
                height: 40
                color: "#f3f4f6"
                radius: 20
                border.color: "#d1d5db"
                border.width: 1

                Row {
                    anchors.fill: parent

                    Rectangle {
                        id: newFileToggle
                        width: parent.width / 2
                        height: parent.height
                        color: !showHistory ? "#4f46e5" : "transparent"
                        radius: 20

                        Text {
                            anchors.centerIn: parent
                            text: "Upload New File"
                            color: !showHistory ? "white" : "#6b7280"
                            font.bold: !showHistory
                            font.pixelSize: 14
                        }

                        MouseArea {
                            anchors.fill: parent
                            onClicked: {
                                showHistory = false
                                console.log("Switched to Upload New File mode")
                            }
                            cursorShape: Qt.PointingHandCursor
                        }
                    }

                    Rectangle {
                        id: historyToggle
                        width: parent.width / 2
                        height: parent.height
                        color: showHistory ? "#4f46e5" : "transparent"
                        radius: 20

                        Text {
                            anchors.centerIn: parent
                            text: "Previous Files"
                            color: showHistory ? "white" : "#6b7280"
                            font.bold: showHistory
                            font.pixelSize: 14
                        }

                        MouseArea {
                            anchors.fill: parent
                            onClicked: {
                                showHistory = true
                                console.log("Switched to Previous Files mode")
                                // Refresh file history when switching to history view
                                fileInputController.refreshFileHistory()
                            }
                            cursorShape: Qt.PointingHandCursor
                        }
                    }
                }
            }
        }

        // Main Content
        Rectangle {
            id: mainContent
            anchors {
                top: toggleSection.bottom
                left: parent.left
                right: parent.right
                bottom: footer.top
                margins: 50
                topMargin: 20
            }
            color: "#ffffff"
            border.width: 2
            border.color: "#d1d5db"
            radius: 10

            // Upload New File Content
            Column {
                id: uploadContent
                anchors.centerIn: parent
                spacing: 20
                visible: !showHistory

                Label {
                    text: "Upload New File"
                    font.pixelSize: 24
                    font.bold: true
                    color: "#1f2937"
                    anchors.horizontalCenter: parent.horizontalCenter
                }

                Label {
                    text: "Upload your course file to start building your schedule"
                    color: "#6b7280"
                    anchors.horizontalCenter: parent.horizontalCenter
                }

                // Upload Area
                Rectangle {
                    id: uploadArea
                    width: 450
                    height: 250
                    color: "#f9fafb"
                    border.width: 2
                    border.color: "#d1d5db"
                    radius: 8

                    DropArea {
                        id: dropArea
                        anchors.fill: parent

                        onEntered: function(drag) {
                            uploadArea.border.color = "#4f46e5"
                            uploadArea.border.width = 3
                        }

                        onExited: function() {
                            uploadArea.border.color = "#d1d5db"
                            uploadArea.border.width = 2
                        }

                        onDropped: function(drop) {
                            uploadArea.border.color = "#d1d5db"
                            uploadArea.border.width = 2

                            if (drop.hasUrls) {
                                let fileUrl = drop.urls[0];
                                let filePath;

                                if (fileUrl.toString().startsWith("file:///")) {
                                    if (fileUrl.toString().match(/^file:\/\/\/[A-Za-z]:/)) {
                                        filePath = fileUrl.toString().replace("file:///", "");
                                    } else {
                                        filePath = fileUrl.toString().replace("file://", "");
                                    }
                                } else {
                                    filePath = fileUrl.toString();
                                }

                                console.log("File dropped:", filePath);
                                fileInputController.handleFileSelected(filePath);
                            } else {
                                showErrorMessage("No valid file was dropped.");
                            }
                        }
                    }

                    Column {
                        anchors.centerIn: parent
                        spacing: 15

                        Text {
                            text: "📁"
                            font.pixelSize: 48
                            color: "#9ca3af"
                            anchors.horizontalCenter: parent.horizontalCenter
                        }

                        Label {
                            id: dropPrompt
                            text: "Drag and drop your file here, or click browse"
                            color: "#6b7280"
                            anchors.horizontalCenter: parent.horizontalCenter
                        }

                        Label {
                            id: fileNameText
                            color: "#4f46e5"
                            font.bold: true
                            visible: false
                            anchors.horizontalCenter: parent.horizontalCenter
                        }

                        Button {
                            id: browseButton
                            anchors.horizontalCenter: parent.horizontalCenter
                            background: Rectangle {
                                color: browseMouseArea.containsMouse ? "#35455c" : "#1f2937"
                                radius: 4
                                implicitWidth: 120
                                implicitHeight: 40
                            }
                            font.bold: true
                            contentItem: Text {
                                text: "Browse Files"
                                color: "white"
                                horizontalAlignment: Text.AlignHCenter
                                verticalAlignment: Text.AlignVCenter
                            }
                            MouseArea {
                                id: browseMouseArea
                                anchors.fill: parent
                                hoverEnabled: true
                                onClicked: {
                                    console.log("Browse button clicked");
                                    fileInputController.handleUploadAndContinue();
                                }
                                cursorShape: Qt.PointingHandCursor
                            }
                        }

                        Label {
                            text: "Supported formats: TXT & XLSX"
                            font.pixelSize: 12
                            color: "#9ca3af"
                            anchors.horizontalCenter: parent.horizontalCenter
                        }
                    }
                }

                Button {
                    id: continueButton
                    visible: false
                    anchors.horizontalCenter: parent.horizontalCenter
                    background: Rectangle {
                        color: generateCoursesMouseArea.containsMouse ? "#35455c" : "#1f2937"
                        radius: 4
                        implicitWidth: 194
                        implicitHeight: 40
                    }
                    font.bold: true
                    contentItem: Text {
                        text: "Upload Course's List →"
                        color: "white"
                        horizontalAlignment: Text.AlignHCenter
                        verticalAlignment: Text.AlignVCenter
                    }
                    MouseArea {
                        id: generateCoursesMouseArea
                        anchors.fill: parent
                        hoverEnabled: true
                        onClicked: {
                            console.log("Continue button clicked - loading new file");
                            fileInputController.loadNewFile();
                        }
                        cursorShape: Qt.PointingHandCursor
                    }
                }
            }

            // History Content
            Column {
                id: historyContent
                anchors.fill: parent
                anchors.margins: 30
                spacing: 20
                visible: showHistory

                Label {
                    text: "Previous Files"
                    font.pixelSize: 24
                    font.bold: true
                    color: "#1f2937"
                    anchors.horizontalCenter: parent.horizontalCenter
                }

                Label {
                    text: "Select from previously uploaded course files"
                    color: "#6b7280"
                    anchors.horizontalCenter: parent.horizontalCenter
                }

                // File List
                ScrollView {
                    Layout.fillWidth: true
                    Layout.fillHeight: true
                    width: parent.width
                    height: parent.height - 200

                    ListView {
                        id: fileHistoryList
                        model: fileInputController.fileHistoryModel
                        spacing: 8

                        // Add empty state
                        Text {
                            anchors.centerIn: parent
                            text: "No files uploaded yet.\nUpload a file first to see history."
                            color: "#6b7280"
                            horizontalAlignment: Text.AlignHCenter
                            visible: fileHistoryList.count === 0
                            font.pixelSize: 14
                        }

                        delegate: Rectangle {
                            id: fileDelegate
                            width: fileHistoryList.width
                            height: 60

                            // Reactive properties for selection state
                            property bool isSelected: fileInputController ? fileInputController.isFileSelected(index) : false

                            color: isSelected ? "#eff6ff" : "#f9fafb"
                            border.color: isSelected ? "#3b82f6" : "#e5e7eb"
                            border.width: isSelected ? 2 : 1
                            radius: 6

                            // Update selection state when the controller's selection changes
                            Connections {
                                target: fileInputController
                                function onFileSelectionChanged() {
                                    fileDelegate.isSelected = fileInputController ? fileInputController.isFileSelected(index) : false
                                }
                            }

                            MouseArea {
                                anchors.fill: parent
                                onClicked: {
                                    console.log("File clicked at index:", index, "File ID:", model.fileId, "File name:", model.fileName);
                                    fileInputController.toggleFileSelection(index);
                                }
                                cursorShape: Qt.PointingHandCursor
                            }

                            Row {
                                anchors.left: parent.left
                                anchors.verticalCenter: parent.verticalCenter
                                anchors.leftMargin: 15
                                spacing: 10

                                Rectangle {
                                    id: checkbox
                                    width: 20
                                    height: 20
                                    radius: 3
                                    color: fileDelegate.isSelected ? "#3b82f6" : "transparent"
                                    border.color: "#3b82f6"
                                    border.width: 2
                                    anchors.verticalCenter: parent.verticalCenter

                                    Text {
                                        text: "✓"
                                        color: "white"
                                        font.pixelSize: 12
                                        anchors.centerIn: parent
                                        visible: fileDelegate.isSelected
                                    }
                                }

                                Column {
                                    anchors.verticalCenter: parent.verticalCenter
                                    spacing: 2

                                    Text {
                                        text: model.fileName || "Unknown File"
                                        color: "#1f2937"
                                        font.pixelSize: 14
                                        font.bold: true
                                    }

                                    Text {
                                        text: `${model.fileType || "Unknown"} • ${model.formattedDate || "Unknown date"}`
                                        color: "#6b7280"
                                        font.pixelSize: 12
                                    }
                                }
                            }
                        }
                    }
                }

                // Selection Info and Load Button
                Column {
                    width: parent.width
                    spacing: 15

                    Label {
                        id: selectedCountText
                        text: ""
                        color: "#6b7280"
                        font.pixelSize: 12
                        anchors.horizontalCenter: parent.horizontalCenter
                    }

                    Row {
                        anchors.horizontalCenter: parent.horizontalCenter
                        spacing: 15

                        Button {
                            text: "Clear Selection"
                            enabled: fileInputController.selectedFileCount > 0
                            background: Rectangle {
                                color: parent.enabled ? (clearMouseArea.containsMouse ? "#f3f4f6" : "#ffffff") : "#f9fafb"
                                border.color: "#d1d5db"
                                border.width: 1
                                radius: 4
                                implicitWidth: 120
                                implicitHeight: 40
                            }
                            contentItem: Text {
                                text: parent.text
                                color: parent.enabled ? "#374151" : "#9ca3af"
                                horizontalAlignment: Text.AlignHCenter
                                verticalAlignment: Text.AlignVCenter
                            }
                            MouseArea {
                                id: clearMouseArea
                                anchors.fill: parent
                                hoverEnabled: true
                                onClicked: {
                                    console.log("Clear selection clicked");
                                    fileInputController.clearFileSelection();
                                }
                                cursorShape: parent.enabled ? Qt.PointingHandCursor : Qt.ArrowCursor
                            }
                        }

                        Button {
                            id: loadHistoryButton
                            text: "Load Selected Files"
                            enabled: false
                            background: Rectangle {
                                color: parent.enabled ? (loadMouseArea.containsMouse ? "#35455c" : "#1f2937") : "#9ca3af"
                                radius: 4
                                implicitWidth: 150
                                implicitHeight: 40
                            }
                            contentItem: Text {
                                text: parent.text
                                color: "white"
                                horizontalAlignment: Text.AlignHCenter
                                verticalAlignment: Text.AlignVCenter
                            }
                            MouseArea {
                                id: loadMouseArea
                                anchors.fill: parent
                                hoverEnabled: true
                                onClicked: {
                                    console.log("Load from history clicked with", fileInputController.selectedFileCount, "files selected");
                                    fileInputController.loadFromHistory();
                                }
                                cursorShape: parent.enabled ? Qt.PointingHandCursor : Qt.ArrowCursor
                            }
                        }
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
            height: 30
            color: "#ffffff"
            border.color: "#e5e7eb"

            Label {
                anchors.centerIn: parent
                text: "© 2025 Schedulify. All rights reserved."
                color: "#6b7280"
                font.pixelSize: 12
            }
        }
    }
}