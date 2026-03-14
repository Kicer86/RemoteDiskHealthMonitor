import QtQuick
import QtQuick.Layouts
import QtQuick.Controls
import RDHM.Monitor

Item {
    id: root

    property string agentName: ""
    property string agentHost: ""
    property int agentPort: 0
    property int agentHealth: 0
    property int agentConnectionState: 3
    property var diskNames: []
    property var diskData: []

    ColumnLayout {
        anchors.fill: parent
        anchors.margins: 16
        spacing: 12

        // Header
        RowLayout {
            Layout.fillWidth: true
            spacing: 12

            StatusBadge {
                healthValue: root.agentHealth
                size: 28
            }

            ColumnLayout {
                Layout.fillWidth: true
                spacing: 2

                Label {
                    text: root.agentName
                    font.bold: true
                    font.pixelSize: 20
                    elide: Text.ElideRight
                    Layout.fillWidth: true
                }

                Label {
                    text: root.agentHost + ":" + root.agentPort
                    font.pixelSize: 12
                    opacity: 0.6
                }
            }

            // Connection status chip
            Rectangle {
                radius: height / 2
                implicitWidth: connStateLabel.implicitWidth + 16
                implicitHeight: connStateLabel.implicitHeight + 8
                color: {
                    switch (root.agentConnectionState) {
                        case 0: return "#FFF3E0"  // Connecting - orange tint
                        case 1: return "#E8F5E9"  // Connected - green tint
                        case 2: return "#FFEBEE"  // Disconnected - red tint
                        case 3: return "#FFEBEE"  // Error - red tint
                        default: return "#F5F5F5"
                    }
                }

                Label {
                    id: connStateLabel
                    anchors.centerIn: parent
                    font.pixelSize: 11
                    text: {
                        switch (root.agentConnectionState) {
                            case 0: return qsTr("Connecting")
                            case 1: return qsTr("Connected")
                            case 2: return qsTr("Disconnected")
                            case 3: return qsTr("Error")
                            default: return "—"
                        }
                    }
                    color: {
                        switch (root.agentConnectionState) {
                            case 0: return "#E65100"
                            case 1: return "#2E7D32"
                            case 2: return "#C62828"
                            case 3: return "#C62828"
                            default: return "#616161"
                        }
                    }
                }
            }
        }

        // Separator
        Rectangle {
            Layout.fillWidth: true
            height: 1
            color: palette.mid
        }

        // Health summary label
        Label {
            text: {
                switch (root.agentHealth) {
                    case HealthEnum.GOOD:         return qsTr("All disks healthy")
                    case HealthEnum.CHECK_STATUS:  return qsTr("Some disks require attention")
                    case HealthEnum.BAD:           return qsTr("Disk failure detected!")
                    default:                       return qsTr("Status unknown")
                }
            }
            font.pixelSize: 13
            font.bold: root.agentHealth === HealthEnum.BAD
            color: {
                switch (root.agentHealth) {
                    case HealthEnum.GOOD:         return "#4CAF50"
                    case HealthEnum.CHECK_STATUS:  return "#FF9800"
                    case HealthEnum.BAD:           return "#F44336"
                    default:                       return palette.text
                }
            }
        }

        // Disks section
        Label {
            text: qsTr("Disks (%1)").arg(root.diskNames.length)
            font.bold: true
            font.pixelSize: 14
            visible: root.diskNames.length > 0
        }

        ListView {
            id: diskListView
            Layout.fillWidth: true
            Layout.fillHeight: true
            clip: true
            spacing: 8

            model: root.diskNames.length

            delegate: Pane {
                id: diskPane
                required property int index

                width: diskListView.width
                padding: 12

                property string diskName: root.diskNames[index] || ""
                property string diskJson: root.diskData[index] || ""
                property var diskObj: diskJson !== "" ? JSON.parse(diskJson) : null
                property bool expanded: false

                background: Rectangle {
                    color: palette.button
                    radius: 6
                    border.color: palette.mid
                    border.width: 1
                }

                contentItem: ColumnLayout {
                    spacing: 8

                    // Disk header - clickable
                    RowLayout {
                        Layout.fillWidth: true
                        spacing: 8

                        Image {
                            source: "qrc:/images/hard-disk.svg"
                            sourceSize: Qt.size(20, 20)
                            Layout.preferredWidth: 20
                            Layout.preferredHeight: 20
                        }

                        Label {
                            text: diskPane.diskName
                            font.bold: true
                            font.pixelSize: 13
                            Layout.fillWidth: true
                        }

                        StatusBadge {
                            healthValue: diskPane.diskObj ? diskPane.diskObj.health : 0
                            size: 20
                        }

                        ToolButton {
                            text: diskPane.expanded ? "▲" : "▼"
                            font.pixelSize: 10
                            Layout.preferredWidth: 28
                            Layout.preferredHeight: 28

                            onClicked: diskPane.expanded = !diskPane.expanded
                        }
                    }

                    // SMART data table (expanded)
                    SmartDataTable {
                        Layout.fillWidth: true
                        visible: diskPane.expanded
                        diskObject: diskPane.diskObj
                    }
                }
            }

            ScrollBar.vertical: ScrollBar {}
        }

        // Empty state for disks
        Label {
            visible: root.diskNames.length === 0
            text: root.agentConnectionState === 1
                ? qsTr("No disk data available")
                : qsTr("Waiting for agent connection...")
            opacity: 0.5
            font.pixelSize: 13
            Layout.fillHeight: true
            Layout.alignment: Qt.AlignHCenter
        }
    }
}
