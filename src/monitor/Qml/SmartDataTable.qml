import QtQuick
import QtQuick.Layouts
import QtQuick.Controls

Item {
    id: root

    property var diskObject: null

    implicitHeight: tableColumn.implicitHeight

    // Extract SMART attributes from the disk object's probes
    function getSmartAttributes() {
        if (!diskObject || !diskObject.probes)
            return []

        var attrs = []
        for (var i = 0; i < diskObject.probes.length; i++) {
            var probe = diskObject.probes[i]
            if (probe.type === "smart" && probe.attributes) {
                for (var j = 0; j < probe.attributes.length; j++) {
                    attrs.push(probe.attributes[j])
                }
            }
        }
        return attrs
    }

    // Get text probes info
    function getTextProbes() {
        if (!diskObject || !diskObject.probes)
            return []

        var texts = []
        for (var i = 0; i < diskObject.probes.length; i++) {
            var probe = diskObject.probes[i]
            if (probe.type === "text" && probe.value) {
                texts.push(probe.value)
            }
        }
        return texts
    }

    ColumnLayout {
        id: tableColumn
        anchors.left: parent.left
        anchors.right: parent.right
        spacing: 2

        // Text probe information
        Repeater {
            model: root.getTextProbes()

            Label {
                required property string modelData
                Layout.fillWidth: true
                text: modelData
                font.pixelSize: 11
                wrapMode: Text.WordWrap
                opacity: 0.7
                padding: 4
            }
        }

        // SMART table header
        Rectangle {
            Layout.fillWidth: true
            height: 28
            color: palette.dark
            radius: 3
            visible: root.getSmartAttributes().length > 0

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
            model: root.getSmartAttributes()

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
                        text: modelData.rawVal !== undefined ? modelData.rawVal : "—"
                        font.pixelSize: 11
                        Layout.preferredWidth: 80
                        horizontalAlignment: Text.AlignRight
                        font.family: "monospace"
                    }
                }
            }
        }

        // No SMART data message
        Label {
            visible: root.getSmartAttributes().length === 0 && root.getTextProbes().length === 0
            text: qsTr("No probe data available")
            font.pixelSize: 11
            opacity: 0.5
            padding: 8
        }
    }
}
