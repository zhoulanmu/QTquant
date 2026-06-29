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
            text: qsTr("行情中心")
            font.pixelSize: 18
            font.bold: true
            color: "#333"
        }
    }

    Rectangle {
        Layout.fillWidth: true
        height: 90
        color: "white"

        RowLayout {
            anchors.fill: parent
            anchors.margins: 16

            ColumnLayout {
                Text {
                    text: "3,256.78"
                    color: "#ff5252"
                    font.pixelSize: 24
                    font.bold: true
                }
                Text {
                    text: qsTr("上证指数")
                    color: "#999"
                    font.pixelSize: 12
                    Layout.topMargin: 4
                }
                Text {
                    text: "+40.12 +1.25%"
                    color: "#ff5252"
                    font.pixelSize: 12
                    Layout.topMargin: 2
                }
            }

            Item { Layout.fillWidth: true }

            ColumnLayout {
                Text {
                    text: "10,876.54"
                    color: "#4caf50"
                    font.pixelSize: 20
                    font.bold: true
                }
                Text {
                    text: qsTr("深证成指")
                    color: "#999"
                    font.pixelSize: 12
                    Layout.topMargin: 4
                }
                Text {
                    text: "-23.45 -0.22%"
                    color: "#4caf50"
                    font.pixelSize: 12
                    Layout.topMargin: 2
                }
            }

            Item { Layout.fillWidth: true }

            ColumnLayout {
                Text {
                    text: "2,156.78"
                    color: "#ff5252"
                    font.pixelSize: 20
                    font.bold: true
                }
                Text {
                    text: qsTr("创业板指")
                    color: "#999"
                    font.pixelSize: 12
                    Layout.topMargin: 4
                }
                Text {
                    text: "+15.67 +0.73%"
                    color: "#ff5252"
                    font.pixelSize: 12
                    Layout.topMargin: 2
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
                        text: qsTr("名称/代码")
                        color: "#999"
                        font.pixelSize: 12
                    }

                    Item { Layout.fillWidth: true }

                    Text {
                        text: qsTr("最新价")
                        color: "#999"
                        font.pixelSize: 12
                        Layout.preferredWidth: 80
                        horizontalAlignment: Text.AlignRight
                    }

                    Text {
                        text: qsTr("涨跌幅")
                        color: "#999"
                        font.pixelSize: 12
                        Layout.preferredWidth: 80
                        horizontalAlignment: Text.AlignRight
                    }
                }
            }

            Repeater {
                model: 20

                Rectangle {
                    Layout.fillWidth: true
                    height: 48
                    color: "white"
                    border.bottom.color: "#f0f0f0"
                    border.bottom.width: 1

                    property string name: ["平安银行", "万科A", "贵州茅台", "招商银行", "中国平安",
                                             "五粮液", "恒瑞医药", "美的集团", "兴业银行", "海康威视",
                                             "比亚迪", "宁德时代", "中信证券", "长江电力", "泸州老窖",
                                             "伊利股份", "紫金矿业", "格力电器", "东方财富", "洋河股份"][index]
                    property string code: ["000001", "000002", "600519", "600036", "601318",
                                            "000858", "600276", "000333", "601166", "002415",
                                            "002594", "300750", "600030", "600900", "000568",
                                            "600887", "601899", "000651", "300059", "002304"][index]
                    property double price: [12.35, 8.76, 1756.00, 35.68, 45.23,
                                             156.78, 42.56, 58.90, 18.45, 32.10,
                                             256.30, 215.80, 19.85, 24.67, 189.50,
                                             28.45, 12.78, 38.90, 15.67, 98.76][index]
                    property double change: [2.35, -1.23, 1.82, -0.56, 3.21,
                                              -2.15, 0.89, 1.45, -0.78, 2.67,
                                              3.21, -0.56, 1.23, -1.89, 2.45,
                                              -0.34, 1.67, -2.34, 0.98, 1.56][index]
                    property bool isUp: change >= 0

                    RowLayout {
                        anchors.fill: parent
                        anchors.leftMargin: 12
                        anchors.rightMargin: 12

                        ColumnLayout {
                            Text {
                                text: parent.parent.name
                                color: "#333"
                                font.pixelSize: 14
                            }
                            Text {
                                text: parent.parent.code
                                color: "#999"
                                font.pixelSize: 11
                            }
                        }

                        Item { Layout.fillWidth: true }

                        Text {
                            text: parent.parent.price.toFixed(2)
                            color: parent.parent.isUp ? "#ff5252" : "#4caf50"
                            font.pixelSize: 14
                            font.bold: true
                            Layout.preferredWidth: 80
                            horizontalAlignment: Text.AlignRight
                        }

                        Text {
                            text: (parent.parent.isUp ? "+" : "") + parent.parent.change.toFixed(2) + "%"
                            color: parent.parent.isUp ? "#ff5252" : "#4caf50"
                            font.pixelSize: 12
                            Layout.preferredWidth: 80
                            horizontalAlignment: Text.AlignRight
                        }
                    }

                    MouseArea {
                        anchors.fill: parent
                        onClicked: {
                            console.log("Clicked:", name)
                        }
                    }
                }
            }
        }
    }
}
