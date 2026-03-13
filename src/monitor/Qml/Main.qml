import QtQuick
import QtQuick.Layouts
import QtQuick.Controls

ApplicationWindow {
    id: root

    width: 900
    height: 600
    minimumWidth: 640
    minimumHeight: 400
    visible: true
    title: qsTr("Remote Disc Health Monitor")

    color: palette.window

    SplitView {
        anchors.fill: parent
        orientation: Qt.Horizontal

        // Left sidebar — agent list
        Pane {
            SplitView.preferredWidth: 280
            SplitView.minimumWidth: 220
            SplitView.maximumWidth: 400
            padding: 0

            ColumnLayout {
                anchors.fill: parent
                spacing: 0

                // Toolbar
                ToolBar {
                    Layout.fillWidth: true

                    RowLayout {
                        anchors.fill: parent
                        anchors.leftMargin: 8
                        anchors.rightMargin: 8

                        Label {
                            text: qsTr("Agents")
                            font.bold: true
                            font.pixelSize: 16
                        }

                        Item { Layout.fillWidth: true }

                        ToolButton {
                            id: addAgentButton
                            text: "+"
                            font.pixelSize: 18
                            font.bold: true

                            ToolTip.visible: hovered
                            ToolTip.text: qsTr("Add agent manually")

                            onClicked: addAgentDialog.open()
                        }
                    }
                }

                // Agent list
                ListView {
                    id: agentListView
                    Layout.fillWidth: true
                    Layout.fillHeight: true
                    clip: true

                    model: agentsModel

                    currentIndex: -1

                    delegate: AgentCard {
                        required property int index
                        required property string agentName
                        required property int agentHealth
                        required property int agentDetectionType
                        required property string agentHost
                        required property int agentPort
                        required property int agentConnectionState
                        required property var agentDiskInfoNames
                        required property var agentDiskInfoData

                        width: agentListView.width
                        highlighted: agentListView.currentIndex === index

                        onClicked: {
                            agentListView.currentIndex = index
                        }

                        onDeleteRequested: {
                            agentsModel.removeAgentAt(index)
                            if (agentListView.currentIndex >= agentsModel.rowCount())
                                agentListView.currentIndex = agentsModel.rowCount() - 1
                        }
                    }

                    // Empty state
                    Label {
                        anchors.centerIn: parent
                        width: parent.width - 40
                        visible: agentListView.count === 0
                        text: qsTr("No agents found.\nAdd one manually or wait\nfor auto-discovery.")
                        horizontalAlignment: Text.AlignHCenter
                        wrapMode: Text.WordWrap
                        opacity: 0.5
                        font.pixelSize: 13
                    }

                    ScrollBar.vertical: ScrollBar {}
                }
            }
        }

        // Right panel — agent detail
        Pane {
            SplitView.fillWidth: true
            SplitView.minimumWidth: 300
            padding: 0

            AgentDetailPanel {
                anchors.fill: parent

                visible: agentListView.currentIndex >= 0

                agentName: agentListView.currentItem ? agentListView.currentItem.agentName : ""
                agentHost: agentListView.currentItem ? agentListView.currentItem.agentHost : ""
                agentPort: agentListView.currentItem ? agentListView.currentItem.agentPort : 0
                agentHealth: agentListView.currentItem ? agentListView.currentItem.agentHealth : 0
                agentConnectionState: agentListView.currentItem ? agentListView.currentItem.agentConnectionState : 3
                diskNames: agentListView.currentItem ? agentListView.currentItem.agentDiskInfoNames : []
                diskData: agentListView.currentItem ? agentListView.currentItem.agentDiskInfoData : []
            }

            // Empty state when no agent selected
            Label {
                anchors.centerIn: parent
                visible: agentListView.currentIndex < 0
                text: qsTr("Select an agent to view details")
                opacity: 0.5
                font.pixelSize: 14
            }
        }
    }

    AddAgentDialog {
        id: addAgentDialog
        parent: Overlay.overlay
    }
}
