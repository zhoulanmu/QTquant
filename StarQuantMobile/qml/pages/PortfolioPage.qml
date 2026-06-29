import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

ColumnLayout {
    id: root
    spacing: 0

    Rectangle {
        Layout.fillWidth: true
        height: 180
        gradient: Gradient {
            GradientStop { position: 0.0; color: "#1565c0" }
            GradientStop { position: 1.0; color: "#42a5f5" }
        }

        ColumnLayout {
            anchors.fill: parent
            anchors.margins: 20

            Text {
                text: qsTr("总资产")
                color: "rgba(255,255,255,0.8)"
                font.pixelSize: 14
            }

            Text {
                text: "¥ 128,567.89"
                color: "white"
                font.pixelSize: 32
                font.bold: true
                Layout.topMargin: 4
            }

            RowLayout {
                Layout.topMargin: 8
                spacing: 24

                ColumnLayout {
                    Text {
                        text: "+¥ 2,345.67"
                        color: "#ff8a80"
                        font.pixelSize: 14
                        font.bold: true
                    }
                    Text {
                        text: qsTr("今日盈亏")
                        color: "rgba(255,255,255,0.7)"
                        font.pixelSize: 11
                        Layout.topMargin: 2
                    }
                }

                ColumnLayout {
                    Text {
                        text: "+1.86%"
                        color: "#ff8a80"
                        font.pixelSize: 14
                        font.bold: true
                    }
                    Text {
                        text: qsTr("收益率")
                        color: "rgba(255,255,255,0.7)"
                        font.pixelSize: 11
                        Layout.topMargin: 2
                    }
                }
            }

            Item { Layout.fillHeight: true }

            RowLayout {
                Layout.fillWidth: true
                spacing: 8

                Rectangle {
                    Layout.fillWidth: true
                    height: 36
                    radius: 18
                    color: "rgba(255,255,255,0.2)"

                    Text {
                        anchors.centerIn: parent
                        text: qsTr("转入")
                        color: "white"
                        font.pixelSize: 13
                    }

                    MouseArea {
                        anchors.fill: parent
                        onClicked: console.log("Transfer in")
                    }
                }

                Rectangle {
                    Layout.fillWidth: true
                    height: 36
                    radius: 18
                    color: "rgba(255,255,255,0.2)"

                    Text {
                        anchors.centerIn: parent
                        text: qsTr("转出")
                        color: "white"
                        font.pixelSize: 13
                    }

                    MouseArea {
                        anchors.fill: parent
                        onClicked: console.log("Transfer out")
                    }
                }

                Rectangle {
                    Layout.fillWidth: true
                    height: 36
                    radius: 18
                    color: "rgba(255,255,255,0.2)"

                    Text {
                        anchors.centerIn: parent
                        text: qsTr("交易")
                        color: "white"
                        font.pixelSize: 13
                    }

                    MouseArea {
                        anchors.fill: parent
                        onClicked: console.log("Trade")
                    }
                }
            }
        }
    }

    Rectangle {
        Layout.fillWidth: true
        height: 72
        color: "white"
        border.bottom.color: "#f0f0f0"
        border.bottom.width: 1

        RowLayout {
            anchors.fill: parent

            ColumnLayout {
                Layout.preferredWidth: parent.width / 4
                Text {
                    text: "¥ 45,678.90"
                    color: "#333"
                    font.pixelSize: 14
                    font.bold: true
                    Layout.alignment: Qt.AlignHCenter
                }
                Text {
                    text: qsTr("可用资金")
                    color: "#999"
                    font.pixelSize: 11
                    Layout.alignment: Qt.AlignHCenter
                    Layout.topMargin: 4
                }
            }

            ColumnLayout {
                Layout.preferredWidth: parent.width / 4
                Text {
                    text: "¥ 82,888.99"
                    color: "#333"
                    font.pixelSize: 14
                    font.bold: true
                    Layout.alignment: Qt.AlignHCenter
                }
                Text {
                    text: qsTr("持仓市值")
                    color: "#999"
                    font.pixelSize: 11
                    Layout.alignment: Qt.AlignHCenter
                    Layout.topMargin: 4
                }
            }

            ColumnLayout {
                Layout.preferredWidth: parent.width / 4
                Text {
                    text: "5"
                    color: "#333"
                    font.pixelSize: 14
                    font.bold: true
                    Layout.alignment: Qt.AlignHCenter
                }
                Text {
                    text: qsTr("持仓数量")
                    color: "#999"
                    font.pixelSize: 11
                    Layout.alignment: Qt.AlignHCenter
                    Layout.topMargin: 4
                }
            }

            ColumnLayout {
                Layout.preferredWidth: parent.width / 4
                Text {
                    text: "+12.5%"
                    color: "#ff5252"
                    font.pixelSize: 14
                    font.bold: true
                    Layout.alignment: Qt.AlignHCenter
                }
                Text {
                    text: qsTr("总收益率")
                    color: "#999"
                    font.pixelSize: 11
                    Layout.alignment: Qt.AlignHCenter
                    Layout.topMargin: 4
                }
            }
        }
    }

    ScrollView {
        Layout.fillWidth: true
        Layout.fillHeight: true
        clip: true

        ColumnLayout {
            width: root.width
            spacing: 0

            Rectangle {
                Layout.fillWidth: true
                height: 36
                color: "#fafafa"

                RowLayout {
                    anchors.fill: parent
                    anchors.leftMargin: 12
                    anchors.rightMargin: 12

                    Text {
                        text: qsTr("我的持仓")
                        color: "#333"
                        font.pixelSize: 14
                        font.bold: true
                    }

                    Item { Layout.fillWidth: true }

                    Text {
                        text: qsTr("全部 >")
                        color: "#1976d2"
                        font.pixelSize: 12
                    }
                }
            }

            Repeater {
                model: 5

                Rectangle {
                    Layout.fillWidth: true
                    height: 56
                    color: "white"
                    border.bottom.color: "#f0f0f0"
                    border.bottom.width: 1

                    property string name: ["贵州茅台", "宁德时代", "比亚迪", "招商银行", "中国平安"][index]
                    property string code: ["600519.SH", "300750.SZ", "002594.SZ", "600036.SH", "601318.SH"][index]
                    property int qty: [10, 100, 50, 200, 300][index]
                    property double cost: [1650.00, 200.50, 240.00, 33.50, 42.80][index]
                    property double current: [1756.00, 215.80, 256.30, 35.68, 45.23][index]
                    property double profit: (current - cost) * qty
                    property double profitPct: (current - cost) / cost * 100
                    property bool isProfit: profit >= 0

                    RowLayout {
                        anchors.fill: parent
                        anchors.leftMargin: 12
                        anchors.rightMargin: 12

                        ColumnLayout {
                            Text {
                                text: parent.parent.name
                                color: "#333"
                                font.pixelSize: 14
                                font.bold: true
                            }
                            Text {
                                text: parent.parent.code + "  " + parent.parent.qty + "股"
                                color: "#999"
                                font.pixelSize: 11
                                Layout.topMargin: 2
                            }
                        }

                        Item { Layout.fillWidth: true }

                        ColumnLayout {
                            Text {
                                text: "¥" + parent.parent.current.toFixed(2)
                                color: "#333"
                                font.pixelSize: 14
                                font.bold: true
                            }
                            Text {
                                text: (parent.parent.isProfit ? "+" : "") + parent.parent.profit.toFixed(2)
                                      + " (" + (parent.parent.isProfit ? "+" : "") + parent.parent.profitPct.toFixed(2) + "%)"
                                color: parent.parent.isProfit ? "#ff5252" : "#4caf50"
                                font.pixelSize: 11
                                Layout.topMargin: 2
                            }
                        }
                    }

                    MouseArea {
                        anchors.fill: parent
                        onClicked: console.log("Clicked holding:", name)
                    }
                }
            }

            Item {
                Layout.fillWidth: true
                Layout.preferredHeight: 20
            }
        }
    }
}
