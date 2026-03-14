import QtQuick
import QtQuick.Layouts
import QtQuick.Controls
import RDHM.Monitor

Item {
    id: root

    property var diskObject: null

    implicitHeight: probesColumn.implicitHeight

    function probeTypeLabel(probe) {
        if (probe.type === "smart")
            return qsTr("SMART Analysis")
        if (probe.type === "text")
            return qsTr("System Log")
        return qsTr("Probe")
    }

    function getProbes() {
        if (!diskObject || !diskObject.probes)
            return []
        return diskObject.probes
    }

    ColumnLayout {
        id: probesColumn
        anchors.left: parent.left
        anchors.right: parent.right
        spacing: 6

        Repeater {
            model: root.getProbes()

            Pane {
                id: probePane
                required property var modelData
                required property int index

                Layout.fillWidth: true
                padding: 0

                property bool probeExpanded: true

                background: Rectangle {
                    color: "transparent"
                    radius: 4
                    border.color: palette.mid
                    border.width: 1
                }

                contentItem: ColumnLayout {
                    spacing: 0

                    // Probe header
                    ItemDelegate {
                        Layout.fillWidth: true
                        implicitHeight: 32
                        padding: 8

                        onClicked: probePane.probeExpanded = !probePane.probeExpanded

                        contentItem: RowLayout {
                            spacing: 6

                            StatusBadge {
                                healthValue: probePane.modelData.health !== undefined ? probePane.modelData.health : 0
                                size: 16
                            }

                            Label {
                                text: root.probeTypeLabel(probePane.modelData)
                                font.bold: true
                                font.pixelSize: 12
                                Layout.fillWidth: true
                            }

                            Label {
                                text: probePane.probeExpanded ? "▲" : "▼"
                                font.pixelSize: 10
                                opacity: 0.5
                            }
                        }
                    }

                    // Probe body
                    Item {
                        Layout.fillWidth: true
                        visible: probePane.probeExpanded
                        implicitHeight: probeBody.implicitHeight

                        ColumnLayout {
                            id: probeBody
                            anchors.left: parent.left
                            anchors.right: parent.right
                            anchors.margins: 8
                            spacing: 2

                            // Self-test status (for SMART probes)
                            Rectangle {
                                Layout.fillWidth: true
                                height: selfTestCol.implicitHeight + 12
                                radius: 4
                                color: Qt.rgba(palette.mid.r, palette.mid.g, palette.mid.b, 0.15)
                                visible: probePane.modelData.type === "smart" && probePane.modelData.selfTestStatus !== undefined

                                ColumnLayout {
                                    id: selfTestCol
                                    anchors.left: parent.left
                                    anchors.right: parent.right
                                    anchors.verticalCenter: parent.verticalCenter
                                    anchors.margins: 8
                                    spacing: 4

                                    RowLayout {
                                        spacing: 6

                                        Label {
                                            text: qsTr("Self-test:")
                                            font.pixelSize: 11
                                            font.bold: true
                                        }

                                        Label {
                                            visible: probePane.modelData.selfTestStatus !== undefined && probePane.modelData.selfTestStatus.running
                                            text: qsTr("Running — %1% remaining").arg(
                                                probePane.modelData.selfTestStatus ? probePane.modelData.selfTestStatus.percentRemaining : 0)
                                            font.pixelSize: 11
                                            color: "#1976D2"
                                        }

                                        Label {
                                            visible: probePane.modelData.selfTestStatus !== undefined && !probePane.modelData.selfTestStatus.running
                                            text: probePane.modelData.selfTestStatus ? (probePane.modelData.selfTestStatus.lastResult || qsTr("No tests run")) : ""
                                            font.pixelSize: 11
                                        }
                                    }

                                    ProgressBar {
                                        Layout.fillWidth: true
                                        visible: probePane.modelData.selfTestStatus !== undefined && probePane.modelData.selfTestStatus.running
                                        from: 0
                                        to: 100
                                        value: probePane.modelData.selfTestStatus ? (100 - probePane.modelData.selfTestStatus.percentRemaining) : 0
                                    }
                                }
                            }

                            // Text probe content
                            Label {
                                visible: probePane.modelData.type === "text" && probePane.modelData.value !== undefined
                                Layout.fillWidth: true
                                text: probePane.modelData.value || ""
                                font.pixelSize: 11
                                wrapMode: Text.WordWrap
                                opacity: 0.8
                                padding: 4
                            }

                            // SMART table header
                            Rectangle {
                                Layout.fillWidth: true
                                height: 28
                                color: palette.dark
                                radius: 3
                                visible: probePane.modelData.type === "smart" && probePane.modelData.attributes !== undefined

                                RowLayout {
                                    anchors.fill: parent
                                    anchors.leftMargin: 8
                                    anchors.rightMargin: 8
                                    spacing: 4

                                    Label {
                                        text: qsTr("Attribute")
                                        font.bold: true
                                        font.pixelSize: 11
                                        Layout.fillWidth: true
                                        Layout.preferredWidth: 200
                                        color: palette.brightText
                                    }
                                    Label {
                                        text: qsTr("Value")
                                        font.bold: true
                                        font.pixelSize: 11
                                        Layout.preferredWidth: 60
                                        horizontalAlignment: Text.AlignRight
                                        color: palette.brightText
                                    }
                                    Label {
                                        text: qsTr("Worst")
                                        font.bold: true
                                        font.pixelSize: 11
                                        Layout.preferredWidth: 60
                                        horizontalAlignment: Text.AlignRight
                                        color: palette.brightText
                                    }
                                    Label {
                                        text: qsTr("Thresh")
                                        font.bold: true
                                        font.pixelSize: 11
                                        Layout.preferredWidth: 60
                                        horizontalAlignment: Text.AlignRight
                                        color: palette.brightText
                                    }
                                    Label {
                                        text: qsTr("Raw")
                                        font.bold: true
                                        font.pixelSize: 11
                                        Layout.preferredWidth: 80
                                        horizontalAlignment: Text.AlignRight
                                        color: palette.brightText
                                    }
                                }
                            }

                            // SMART attribute rows
                            Repeater {
                                model: probePane.modelData.type === "smart" ? (probePane.modelData.attributes || []) : []

                                Rectangle {
                                    required property var modelData
                                    required property int index
                                    Layout.fillWidth: true
                                    height: 24
                                    radius: 2
                                    color: index % 2 === 0 ? "transparent" : Qt.rgba(palette.mid.r, palette.mid.g, palette.mid.b, 0.2)

                                    RowLayout {
                                        anchors.fill: parent
                                        anchors.leftMargin: 8
                                        anchors.rightMargin: 8
                                        spacing: 4

                                        Label {
                                            text: modelData.name || ""
                                            font.pixelSize: 11
                                            Layout.fillWidth: true
                                            Layout.preferredWidth: 200
                                            elide: Text.ElideRight
                                        }
                                        Label {
                                            text: modelData.value !== undefined ? modelData.value : "—"
                                            font.pixelSize: 11
                                            Layout.preferredWidth: 60
                                            horizontalAlignment: Text.AlignRight
                                            font.family: "monospace"
                                        }
                                        Label {
                                            text: modelData.worst !== undefined ? modelData.worst : "—"
                                            font.pixelSize: 11
                                            Layout.preferredWidth: 60
                                            horizontalAlignment: Text.AlignRight
                                            font.family: "monospace"
                                        }
                                        Label {
                                            text: modelData.threshold !== undefined ? modelData.threshold : "—"
                                            font.pixelSize: 11
                                            Layout.preferredWidth: 60
                                            horizontalAlignment: Text.AlignRight
                                            font.family: "monospace"
                                            color: modelData.threshold > 0 && modelData.value <= modelData.threshold ? "red" : palette.text
                                        }
                                        Label {
                                            text: modelData.rawVal !== undefined ? modelData.rawVal : "—"
                                            font.pixelSize: 11
                                            Layout.preferredWidth: 80
                                            horizontalAlignment: Text.AlignRight
                                            font.family: "monospace"
                                        }
                                    }
                                }
                            }

                            // No data message for this probe
                            Label {
                                visible: {
                                    if (probePane.modelData.type === "smart")
                                        return !probePane.modelData.attributes || probePane.modelData.attributes.length === 0
                                    if (probePane.modelData.type === "text")
                                        return !probePane.modelData.value
                                    return true
                                }
                                text: qsTr("No data available")
                                font.pixelSize: 11
                                opacity: 0.5
                                padding: 4
                            }

                            // Bottom spacing
                            Item { height: 4 }
                        }
                    }
                }
            }
        }

        // No probes at all
        Label {
            visible: root.getProbes().length === 0
            text: qsTr("No probe data available")
            font.pixelSize: 11
            opacity: 0.5
            padding: 8
        }
    }
}
