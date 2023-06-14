
import QtQuick 2.15
import QtQuick.Layouts 1.15
import QtQuick.Controls 2.15

Item {
    id: mainView

    width: 390
    height: 520

    signal newAgentRequested(string name, string ip, string port)

    state: "NormalState"

    ColumnLayout {
        id: column
        anchors.fill: parent

        AgentsView {
            id: agentsView
            objectName: "activeAgents"

            Layout.fillWidth: true
            Layout.fillHeight: true

            label: qsTr("Agents:")
        }

        Button {
            id: button
            text: qsTr("Add agent...")

            onClicked: mainView.state = mainView.state === "NormalState"? "AddingAgentState": "NormalState"
        }

        GridLayout {
            id: grid

            Layout.fillHeight: true
            Layout.fillWidth: false

            Layout.margins: 5
            columns: 2

            Text {
                text: qsTr("Name:")
                font.pixelSize: 12
            }

            TextField {
                id: nameInput

                font.pixelSize: 12
                selectByMouse: true
            }

            Text {
                text: qsTr("IP:")
                font.pixelSize: 12
            }

            TextField {
                id: ipInput

                validator: RegularExpressionValidator { regularExpression: /([0-9]{1,3}\.){3}[0-9]{1,3}/ }
                font.pixelSize: 12
                selectByMouse: true
            }

            Text {
                text: qsTr("Port:")
                font.pixelSize: 12
            }

            TextField {
                id: portInput

                text: "1630"

                validator: IntValidator { top: 65535; bottom: 0; }
                font.pixelSize: 12
                selectByMouse: true
            }

            Item {
                width: 10
                height: 10
            }

            Button {
                id: addNew
                text: qsTr("Add")
                Layout.alignment: Qt.AlignRight | Qt.AlignVCenter

                onClicked: mainView.newAgentRequested(nameInput.text, ipInput.text, portInput.text)
            }
        }
    }

    states: [
        State {
            name: "NormalState"

            PropertyChanges {
                target: grid
                visible: false
            }
        },
        State {
            name: "AddingAgentState"

            PropertyChanges {
                target: button
                text: qsTr("Cancel")
            }
        }
    ]
}
