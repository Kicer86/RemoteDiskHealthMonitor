import QtQuick
import QtQuick.Layouts
import QtQuick.Controls
import RDHM.Monitor

ItemDelegate {
    id: card

    signal deleteRequested()

    required property int agentHealth
    required property int agentDetectionType
    required property int agentConnectionState
    required property string agentName
    required property string agentHost
    required property int agentPort

    height: 64
    padding: 8

    contentItem: RowLayout {
        spacing: 10

        // Detection source icon
        Image {
            Layout.preferredWidth: 24
            Layout.preferredHeight: 24
            Layout.alignment: Qt.AlignVCenter

            source: agentDetectionType === AgentInformation.Hardcoded
                ? "qrc:/images/hard-disk2.svg"
                : "qrc:/images/hard-disk.svg"
            sourceSize: Qt.size(24, 24)
            smooth: true
        }

        // Agent info
        ColumnLayout {
            Layout.fillWidth: true
            spacing: 2

            Label {
                text: agentName
                font.bold: true
                font.pixelSize: 13
                elide: Text.ElideRight
                Layout.fillWidth: true
            }

            Label {
                text: agentHost + ":" + agentPort
                font.pixelSize: 11
                opacity: 0.6
                elide: Text.ElideRight
                Layout.fillWidth: true
            }
        }

        // Connection state indicator
        Label {
            visible: agentConnectionState !== 1  // not Connected
            text: {
                switch (agentConnectionState) {
                    case 0: return qsTr("Connecting...")
                    case 2: return qsTr("Offline")
                    case 3: return qsTr("Error")
                    default: return ""
                }
            }
            font.pixelSize: 10
            font.italic: true
            opacity: 0.6
        }

        // Health badge
        StatusBadge {
            healthValue: agentHealth
        }

        // Delete button for hardcoded agents
        ToolButton {
            visible: agentDetectionType === AgentInformation.Hardcoded && card.hovered
            icon.source: "qrc:/images/trash.svg"
            icon.width: 16
            icon.height: 16
            Layout.preferredWidth: 28
            Layout.preferredHeight: 28

            ToolTip.visible: hovered
            ToolTip.text: qsTr("Remove agent")

            onClicked: card.deleteRequested()
        }
    }
}
