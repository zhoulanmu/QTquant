import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

ColumnLayout {
    id: root
    spacing: 0

    Rectangle {
        Layout.fillWidth: true
        height: 48
        color: "white"
        border.bottom.color: "#e0e0e0"
        border.bottom.width: 1

        Text {
            anchors.centerIn: parent
            text: qsTr("智能选股")
            font.pixelSize: 18
            font.bold: true
            color: "#333"
        }
    }

    ScrollView {
        Layout.fillWidth: true
        Layout.fillHeight: true
        clip: true

        ColumnLayout {
            width: root.width
            spacing: 12

            Row {
                Layout.fillWidth: true
                Layout.leftMargin: 12
                Layout.rightMargin: 12
                Layout.topMargin: 12
                spacing: 8

                Repeater {
                    model: stockScreener ? stockScreener.presetStrategies : []

                    Rectangle {
                        width: (parent.width - 24) / 4 - 6
                        height: 36
                        radius: 18
                        color: index < 4 ? "#e3f2fd" : "#f5f5f5"
                        border.color: index < 4 ? "#1976d2" : "#ddd"
                        border.width: 1

                        Text {
                            anchors.centerIn: parent
                            text: modelData
                            color: index < 4 ? "#1976d2" : "#666"
                            font.pixelSize: 12
                            elide: Text.ElideRight
                            maximumLineCount: 1
                        }

                        MouseArea {
                            anchors.fill: parent
                            onClicked: {
                                if (stockScreener) {
                                    stockScreener.applyPreset(modelData)
                                    stockScreener.startScreener()
                                }
                            }
                        }
                    }
                }
            }

            Row {
                Layout.fillWidth: true
                Layout.leftMargin: 12
                Layout.rightMargin: 12
                Layout.topMargin: 4
                spacing: 8

                Repeater {
                    model: 4

                    Rectangle {
                        width: (parent.width - 24) / 4 - 6
                        height: 36
                        radius: 18
                        color: "#f5f5f5"
                        border.color: "#ddd"
                        border.width: 1

                        property string label: ["均线多头", "MACD金叉", "突破平台", "超跌反弹"][index]

                        Text {
                            anchors.centerIn: parent
                            text: parent.label
                            color: "#666"
                            font.pixelSize: 12
                        }

                        MouseArea {
                            anchors.fill: parent
                            onClicked: {
                                if (stockScreener) {
                                    stockScreener.applyPreset(parent.label)
                                    stockScreener.startScreener()
                                }
                            }
                        }
                    }
                }
            }

            Rectangle {
                Layout.fillWidth: true
                Layout.leftMargin: 12
                Layout.rightMargin: 12
                Layout.topMargin: 8
                height: 44
                radius: 8
                color: "white"

                RowLayout {
                    anchors.fill: parent
                    anchors.leftMargin: 12
                    anchors.rightMargin: 12

                    Text {
                        text: qsTr("筛选结果")
                        color: "#333"
                        font.pixelSize: 14
                        font.bold: true
                    }

                    Item { Layout.fillWidth: true }

                    Text {
                        text: stockScreener ? (stockScreener.resultCount + " 只") : "0 只"
                        color: "#1976d2"
                        font.pixelSize: 13
                        font.bold: true
                    }
                }
            }

            ColumnLayout {
                Layout.fillWidth: true
                Layout.leftMargin: 12
                Layout.rightMargin: 12
                Layout.bottomMargin: 12
                spacing: 6

                Repeater {
                    model: stockScreener ? stockScreener.resultCount : 0

                    delegate: Rectangle {
                        Layout.fillWidth: true
                        height: 60
                        radius: 8
                        color: "white"

                        RowLayout {
                            anchors.fill: parent
                            anchors.margins: 12

                            ColumnLayout {
                                Text {
                                    text: stockScreener && stockScreener.results[index] ? stockScreener.results[index].name : "加载中"
                                    color: "#333"
                                    font.pixelSize: 14
                                    font.bold: true
                                }
                                Text {
                                    text: stockScreener && stockScreener.results[index] ? stockScreener.results[index].symbol : ""
                                    color: "#999"
                                    font.pixelSize: 11
                                    Layout.topMargin: 2
                                }
                            }

                            Item { Layout.fillWidth: true }

                            ColumnLayout {
                                Text {
                                    text: stockScreener && stockScreener.results[index] ?
                                              stockScreener.results[index].price.toFixed(2) : "--"
                                    color: stockScreener && stockScreener.results[index] && stockScreener.results[index].changePercent >= 0 ?
                                               "#ff5252" : "#4caf50"
                                    font.pixelSize: 14
                                    font.bold: true
                                }
                                Text {
                                    text: stockScreener && stockScreener.results[index] ?
                                              (stockScreener.results[index].changePercent >= 0 ? "+" : "") +
                                              stockScreener.results[index].changePercent.toFixed(2) + "%" : "--"
                                    color: stockScreener && stockScreener.results[index] && stockScreener.results[index].changePercent >= 0 ?
                                               "#ff5252" : "#4caf50"
                                    font.pixelSize: 11
                                    Layout.topMargin: 2
                                }
                            }
                        }

                        MouseArea {
                            anchors.fill: parent
                            onClicked: {
                                console.log("Clicked stock at index", index)
                            }
                        }
                    }
                }

                Rectangle {
                    Layout.fillWidth: true
                    height: 44
                    radius: 8
                    color: "#e3f2fd"
                    visible: stockScreener && stockScreener.isRunning

                    Text {
                        anchors.centerIn: parent
                        text: qsTr("正在筛选中...")
                        color: "#1976d2"
                        font.pixelSize: 13
                    }
                }
            }
        }
    }
}
