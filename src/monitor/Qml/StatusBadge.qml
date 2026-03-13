import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import RDHM.Monitor

Rectangle {
    id: badge

    property int healthValue: HealthEnum.UNKNOWN
    property int size: 22

    width: size
    height: size
    radius: size / 2

    color: {
        switch (healthValue) {
            case HealthEnum.GOOD:         return "#4CAF50"
            case HealthEnum.CHECK_STATUS: return "#FF9800"
            case HealthEnum.BAD:          return "#F44336"
            default:                      return "#9E9E9E"
        }
    }

    Image {
        anchors.centerIn: parent
        width: parent.width * 0.6
        height: parent.height * 0.6
        sourceSize: Qt.size(width, height)
        smooth: true

        source: {
            switch (badge.healthValue) {
                case HealthEnum.GOOD:         return "qrc:/images/check.svg"
                case HealthEnum.CHECK_STATUS: return "qrc:/images/warning.svg"
                case HealthEnum.BAD:          return "qrc:/images/close.svg"
                default:                      return "qrc:/images/question-mark.svg"
            }
        }
    }

    ToolTip {
        id: tooltip
        visible: mouseArea.containsMouse
        text: {
            switch (badge.healthValue) {
                case HealthEnum.GOOD:         return qsTr("Good")
                case HealthEnum.CHECK_STATUS: return qsTr("Check Status")
                case HealthEnum.BAD:          return qsTr("Bad")
                default:                      return qsTr("Unknown")
            }
        }
    }

    MouseArea {
        id: mouseArea
        anchors.fill: parent
        hoverEnabled: true
        acceptedButtons: Qt.NoButton
    }
}
