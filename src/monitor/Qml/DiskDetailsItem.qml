
import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import Qt.labs.qmlmodels

TableView {
    id: tableView
    Layout.fillHeight: true
    Layout.fillWidth: true
    Layout.rowSpan: 5
    Layout.columnSpan: 1
    Layout.alignment: Qt.AlignLeft | Qt.AlignTop
    columnSpacing: 1
    rowSpacing: 1
    clip: true
    visible: false
    boundsMovement: Flickable.StopAtBounds

    columnWidthProvider: columnWidthFun
    rowHeightProvider: rowHeightFun

    onWidthChanged: tableView.forceLayout();
    onHeightChanged: tableView.forceLayout()

    ScrollBar.vertical: ScrollBar {
        anchors.right: parent.right
        visible: tableView.contentHeight > tableView.height
    }

    ScrollBar.horizontal: ScrollBar {
        anchors.bottom: parent.bottom
        visible: tableView.contentWidth > tableView.width
    }

    model: TableModel {
        id: tableModel
        TableModelColumn { display: "attr" }
        TableModelColumn { display: "value" }
        TableModelColumn { display: "worst" }
        TableModelColumn { display: "rawVal" }

        rows: [{
            attr: "Attribute Name",
            value: "Value",
            worst: "Worst",
            rawVal: "Raw Value",
        }]
    }

    delegate: Rectangle {
        id: delegateRectangle
        required property var row
        implicitWidth: 100
        border.width: 0
        color: {
            if (row % 2)
                {
                    return "light gray"
                } else
                {
                    return "gray"
                }
                }

        Text {
            text: display
            anchors.centerIn: parent
        }
    }

    function rowHeightFun(row) {
        var txt = tableModel.rows[row].attr
        metrics.text = txt
        var textHeight = metrics.boundingRect.height

        return textHeight + 4
    }

    function columnWidthFun(column) {
        if (column === 0) {
            var rows = tableView.model.rowCount
            var maxWidth = 0;

            for(var i = 0; i < rows; i++) {
                var modelObject = tableView.model.getRow(i)
                var txt = modelObject.attr
                metrics.text = txt
                var textWidth = metrics.boundingRect.width

                if (textWidth > maxWidth)
                    maxWidth = textWidth;
            }

            return maxWidth + 10;   // some margin

        } else {
            metrics.text = tableView.model.getRow(i).rawVal
            return ( metrics.boundingRect.width + 10 );
        }
    }

}
