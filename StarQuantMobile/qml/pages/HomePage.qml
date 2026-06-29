import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

ScrollView {
    id: root
    clip: true

    ColumnLayout {
        width: root.width
        spacing: 12

        Rectangle {
            Layout.fillWidth: true
            Layout.margins: 12
            Layout.topMargin: 16
            height: 120
            radius: 12
            gradient: Gradient {
                GradientStop { position: 0.0; color: "#1976d2" }
                GradientStop { position: 1.0; color: "#42a5f5" }
            }

            ColumnLayout {
                anchors.fill: parent
                anchors.margins: 16

                Text {
                    text: qsTr("星策 StarQuant")
                    color: "white"
                    font.pixelSize: 20
                    font.bold: true
                }

                Text {
                    text: qsTr("先以模拟谋方略，再持真仓逐行情")
                    color: "rgba(255,255,255,0.8)"
                    font.pixelSize: 12
                    Layout.topMargin: 4
                }

                Item { Layout.fillHeight: true }

                RowLayout {
                    Layout.fillWidth: true

                    ColumnLayout {
                        Text {
                            text: "3,256.78"
                            color: "white"
                            font.pixelSize: 18
                            font.bold: true
                        }
                        Text {
                            text: qsTr("上证指数")
                            color: "rgba(255,255,255,0.7)"
                            font.pixelSize: 10
                        }
                    }

                    Item { Layout.fillWidth: true }

                    ColumnLayout {
                        Text {
                            text: "+1.25%"
                            color: "#ff5252"
                            font.pixelSize: 18
                            font.bold: true
                        }
                        Text {
                            text: qsTr("涨跌幅")
                            color: "rgba(255,255,255,0.7)"
                            font.pixelSize: 10
                        }
                    }
                }
            }
        }

        RowLayout {
            Layout.fillWidth: true
            Layout.leftMargin: 12
            Layout.rightMargin: 12
            spacing: 8

            Rectangle {
                Layout.fillWidth: true
                Layout.preferredHeight: 80
                radius: 8
                color: "white"

                ColumnLayout {
                    anchors.centerIn: parent

                    Text {
                        text: qsTr("选股")
                        color: "#333"
                        font.pixelSize: 14
                        font.bold: true
                        Layout.alignment: Qt.AlignHCenter
                    }
                    Text {
                        text: qsTr("智能筛选")
                        color: "#999"
                        font.pixelSize: 11
                        Layout.alignment: Qt.AlignHCenter
                        Layout.topMargin: 4
                    }
                }
            }

            Rectangle {
                Layout.fillWidth: true
                Layout.preferredHeight: 80
                radius: 8
                color: "white"

                ColumnLayout {
                    anchors.centerIn: parent

                    Text {
                        text: qsTr("策略")
                        color: "#333"
                        font.pixelSize: 14
                        font.bold: true
                        Layout.alignment: Qt.AlignHCenter
                    }
                    Text {
                        text: qsTr("量化交易")
                        color: "#999"
                        font.pixelSize: 11
                        Layout.alignment: Qt.AlignHCenter
                        Layout.topMargin: 4
                    }
                }
            }

            Rectangle {
                Layout.fillWidth: true
                Layout.preferredHeight: 80
                radius: 8
                color: "white"

                ColumnLayout {
                    anchors.centerIn: parent

                    Text {
                        text: qsTr("行情")
                        color: "#333"
                        font.pixelSize: 14
                        font.bold: true
                        Layout.alignment: Qt.AlignHCenter
                    }
                    Text {
                        text: qsTr("实时数据")
                        color: "#999"
                        font.pixelSize: 11
                        Layout.alignment: Qt.AlignHCenter
                        Layout.topMargin: 4
                    }
                }
            }
        }

        RowLayout {
            Layout.fillWidth: true
            Layout.leftMargin: 12
            Layout.rightMargin: 12
            spacing: 8

            Rectangle {
                Layout.fillWidth: true
                Layout.preferredHeight: 80
                radius: 8
                color: "white"

                ColumnLayout {
                    anchors.centerIn: parent

                    Text {
                        text: qsTr("持仓")
                        color: "#333"
                        font.pixelSize: 14
                        font.bold: true
                        Layout.alignment: Qt.AlignHCenter
                    }
                    Text {
                        text: qsTr("资产明细")
                        color: "#999"
                        font.pixelSize: 11
                        Layout.alignment: Qt.AlignHCenter
                        Layout.topMargin: 4
                    }
                }
            }

            Rectangle {
                Layout.fillWidth: true
                Layout.preferredHeight: 80
                radius: 8
                color: "white"

                ColumnLayout {
                    anchors.centerIn: parent

                    Text {
                        text: qsTr("资讯")
                        color: "#333"
                        font.pixelSize: 14
                        font.bold: true
                        Layout.alignment: Qt.AlignHCenter
                    }
                    Text {
                        text: qsTr("市场快讯")
                        color: "#999"
                        font.pixelSize: 11
                        Layout.alignment: Qt.AlignHCenter
                        Layout.topMargin: 4
                    }
                }
            }

            Rectangle {
                Layout.fillWidth: true
                Layout.preferredHeight: 80
                radius: 8
                color: "white"

                ColumnLayout {
                    anchors.centerIn: parent

                    Text {
                        text: qsTr("设置")
                        color: "#333"
                        font.pixelSize: 14
                        font.bold: true
                        Layout.alignment: Qt.AlignHCenter
                    }
                    Text {
                        text: qsTr("个性化")
                        color: "#999"
                        font.pixelSize: 11
                        Layout.alignment: Qt.AlignHCenter
                        Layout.topMargin: 4
                    }
                }
            }
        }

        Item {
            Layout.fillWidth: true
            Layout.preferredHeight: 1
            Layout.leftMargin: 12
            Layout.rightMargin: 12
            Rectangle {
                anchors.fill: parent
                color: "#e0e0e0"
            }
        }

        Row {
            Layout.fillWidth: true
            Layout.leftMargin: 12
            Layout.rightMargin: 12
            Layout.topMargin: 8

            Text {
                text: qsTr("热门股票")
                color: "#333"
                font.pixelSize: 16
                font.bold: true
            }

            Item { width: parent.width - text1.width - text2.width }

            Text {
                text: qsTr("更多 >")
                color: "#1976d2"
                font.pixelSize: 12
            }
        }

        ColumnLayout {
            Layout.fillWidth: true
            Layout.leftMargin: 12
            Layout.rightMargin: 12
            Layout.topMargin: 8
            spacing: 8

            Repeater {
                model: 5

                Rectangle {
                    Layout.fillWidth: true
                    height: 56
                    radius: 8
                    color: "white"

                    RowLayout {
                        anchors.fill: parent
                        anchors.margins: 12

                        ColumnLayout {
                            Text {
                                text: ["平安银行", "贵州茅台", "宁德时代", "比亚迪", "招商银行"][index]
                                color: "#333"
                                font.pixelSize: 14
                                font.bold: true
                            }
                            Text {
                                text: ["000001.SZ", "600519.SH", "300750.SZ", "002594.SZ", "600036.SH"][index]
                                color: "#999"
                                font.pixelSize: 11
                                Layout.topMargin: 2
                            }
                        }

                        Item { Layout.fillWidth: true }

                        ColumnLayout {
                            Text {
                                text: ["12.35", "1756.00", "215.80", "256.30", "35.68"][index]
                                color: ["#ff5252", "#ff5252", "#4caf50", "#ff5252", "#4caf50"][index]
                                font.pixelSize: 14
                                font.bold: true
                            }
                            Text {
                                text: ["+2.35%", "+1.82%", "-0.56%", "+3.21%", "-1.23%"][index]
                                color: ["#ff5252", "#ff5252", "#4caf50", "#ff5252", "#4caf50"][index]
                                font.pixelSize: 11
                                Layout.topMargin: 2
                            }
                        }
                    }
                }
            }
        }

        Item { Layout.fillHeight: true }
    }
}
