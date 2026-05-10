import QtQuick
import QtQuick.Layouts
import QtQuick.Controls

Dialog {
    id: dialog

    title: qsTr("Add Agent")

    modal: true
    anchors.centerIn: parent
    width: 360

    function resetFields() {
        nameField.text = ""
        ipField.text = ""
        portField.text = "1630"
        errorLabel.text = ""
    }

    function submitAgent() {
        errorLabel.text = ""
        agentsValidator.addNewAgent(nameField.text, ipField.text, portField.text)
    }

    onRejected: resetFields()

    Connections {
        target: agentsValidator

        function onValidationFailed(reason) {
            errorLabel.text = reason
        }

        function onAgentDiscovered() {
            dialog.resetFields()
            dialog.close()
        }
    }

    footer: DialogButtonBox {
        Button {
            text: qsTr("OK")
            DialogButtonBox.buttonRole: DialogButtonBox.ActionRole
            onClicked: dialog.submitAgent()
        }

        Button {
            text: qsTr("Cancel")
            DialogButtonBox.buttonRole: DialogButtonBox.RejectRole
            onClicked: dialog.reject()
        }
    }

    contentItem: GridLayout {
        columns: 2
        columnSpacing: 12
        rowSpacing: 8

        Label { text: qsTr("Name:") }
        TextField {
            id: nameField
            Layout.fillWidth: true
            placeholderText: qsTr("My Agent")
            selectByMouse: true
        }

        Label { text: qsTr("IP Address:") }
        TextField {
            id: ipField
            Layout.fillWidth: true
            placeholderText: "192.168.1.100"
            validator: RegularExpressionValidator {
                regularExpression: /([0-9]{1,3}\.){3}[0-9]{1,3}/
            }
            selectByMouse: true
        }

        Label { text: qsTr("Port:") }
        TextField {
            id: portField
            Layout.fillWidth: true
            text: "1630"
            validator: IntValidator { bottom: 1; top: 65535 }
            selectByMouse: true
        }

        // Error message
        Label {
            id: errorLabel
            Layout.columnSpan: 2
            Layout.fillWidth: true
            color: "#F44336"
            font.pixelSize: 12
            wrapMode: Text.WordWrap
            visible: text.length > 0
        }
    }
}
