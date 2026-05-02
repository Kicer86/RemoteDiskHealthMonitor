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
    property string lastRefreshed: ""

    // TODO: try using funcion from 'common' module
    function formatBytes(bytes) {
        if (bytes <= 0) return ""
        var units = ["B", "KB", "MB", "GB", "TB", "PB"]
        var i = 0
        var val = bytes
        while (val >= 1024 && i < units.length - 1) {
            val /= 1024
            i++
        }
        return val.toFixed(i > 0 ? 1 : 0) + " " + units[i]
    }

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

        // Last refreshed
        Label {
            visible: root.lastRefreshed !== ""
            text: qsTr("Last refreshed: %1").arg(root.lastRefreshed)
            font.pixelSize: 11
            opacity: 0.5
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

                    // Disk summary info
                    ColumnLayout {
                        Layout.fillWidth: true
                        spacing: 2
                        visible: diskPane.diskObj && diskPane.diskObj.summary !== undefined

                        property var s: diskPane.diskObj ? diskPane.diskObj.summary : null

                        // Row 1: model · vendor · capacity
                        RowLayout {
                            Layout.fillWidth: true
                            spacing: 0
                            visible: parent.s && (parent.s.model !== "" || parent.s.vendor !== "" || parent.s.capacityBytes > 0)

                            Label {
                                visible: parent.parent.s && parent.parent.s.model !== ""
                                text: parent.parent.s ? parent.parent.s.model : ""
                                font.pixelSize: 11
                                opacity: 0.7
                            }

                            Label {
                                visible: parent.parent.s && parent.parent.s.model !== "" && parent.parent.s.vendor !== ""
                                text: "  ·  "
                                font.pixelSize: 11
                                opacity: 0.4
                            }

                            Label {
                                visible: parent.parent.s && parent.parent.s.vendor !== ""
                                text: parent.parent.s ? parent.parent.s.vendor : ""
                                font.pixelSize: 11
                                opacity: 0.7
                            }

                            Label {
                                visible: parent.parent.s && parent.parent.s.capacityBytes > 0 && (parent.parent.s.model !== "" || parent.parent.s.vendor !== "")
                                text: "  ·  "
                                font.pixelSize: 11
                                opacity: 0.4
                            }

                            Label {
                                visible: parent.parent.s && parent.parent.s.capacityBytes > 0
                                text: parent.parent.s ? root.formatBytes(parent.parent.s.capacityBytes) : ""
                                font.pixelSize: 11
                                opacity: 0.7
                            }

                            Item { Layout.fillWidth: true }
                        }

                        // Row 2: drive type badge · temperature · power-on hours
                        RowLayout {
                            Layout.fillWidth: true
                            spacing: 0
                            visible: parent.s && (parent.s.driveType !== "" || parent.s.temperatureC !== undefined || parent.s.powerOnHours !== undefined)

                            Rectangle {
                                visible: parent.parent.s && parent.parent.s.driveType !== ""
                                width: driveTypeLabel.implicitWidth + 10
                                height: driveTypeLabel.implicitHeight + 4
                                radius: 3
                                color: {
                                    if (!parent.parent.s) return "#9E9E9E"
                                    switch (parent.parent.s.driveType) {
                                        case "NVMe": return "#1565C0"
                                        case "SSD":  return "#2E7D32"
                                        case "HDD":  return "#795548"
                                        case "USB":  return "#F57C00"
                                        default:     return "#9E9E9E"
                                    }
                                }

                                Label {
                                    id: driveTypeLabel
                                    anchors.centerIn: parent
                                    text: parent.parent.parent.s ? parent.parent.parent.s.driveType : ""
                                    font.pixelSize: 10
                                    font.bold: true
                                    color: "white"
                                }
                            }

                            Label {
                                visible: parent.parent.s && parent.parent.s.driveType !== "" && parent.parent.s.temperatureC !== undefined
                                text: "  ·  "
                                font.pixelSize: 11
                                opacity: 0.4
                            }

                            Label {
                                visible: parent.parent.s && parent.parent.s.temperatureC !== undefined
                                text: parent.parent.s && parent.parent.s.temperatureC !== undefined ? parent.parent.s.temperatureC + " °C" : ""
                                font.pixelSize: 11
                                opacity: 0.7
                            }

                            Label {
                                visible: parent.parent.s && parent.parent.s.powerOnHours !== undefined && (parent.parent.s.driveType !== "" || parent.parent.s.temperatureC !== undefined)
                                text: "  ·  "
                                font.pixelSize: 11
                                opacity: 0.4
                            }

                            Label {
                                visible: parent.parent.s && parent.parent.s.powerOnHours !== undefined
                                text: parent.parent.s && parent.parent.s.powerOnHours !== undefined ? qsTr("%1 h on").arg(parent.parent.s.powerOnHours) : ""
                                font.pixelSize: 11
                                opacity: 0.7
                            }

                            Item { Layout.fillWidth: true }
                        }
                    }

                    // Probe details (expanded)
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
