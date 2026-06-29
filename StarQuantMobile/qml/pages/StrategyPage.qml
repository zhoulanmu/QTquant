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
            text: qsTr("量化策略")
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

            Rectangle {
                Layout.fillWidth: true
                Layout.leftMargin: 12
                Layout.rightMargin: 12
                Layout.topMargin: 12
                height: 120
                radius: 10
                gradient: Gradient {
                    GradientStop { position: 0.0; color: "#ff6f00" }
                    GradientStop { position: 1.0; color: "#ffa726" }
                }

                ColumnLayout {
                    anchors.fill: parent
                    anchors.margins: 16

                    RowLayout {
                        Text {
                            text: qsTr("双均线策略")
                            color: "white"
                            font.pixelSize: 16
                            font.bold: true
                        }
                        Item { Layout.fillWidth: true }
                        Rectangle {
                            width: 56
                            height: 24
                            radius: 12
                            color: "rgba(255,255,255,0.25)"

                            Text {
                                anchors.centerIn: parent
                                text: qsTr("运行中")
                                color: "white"
                                font.pixelSize: 11
                            }
                        }
                    }

                    Text {
                        text: qsTr("快速均线与慢速均线金叉死叉交易策略")
                        color: "rgba(255,255,255,0.8)"
                        font.pixelSize: 12
                        Layout.topMargin: 4
                    }

                    Item { Layout.fillHeight: true }

                    RowLayout {
                        Layout.fillWidth: true

                        ColumnLayout {
                            Text {
                                text: "15.8%"
                                color: "white"
                                font.pixelSize: 18
                                font.bold: true
                            }
                            Text {
                                text: qsTr("年化收益")
                                color: "rgba(255,255,255,0.7)"
                                font.pixelSize: 10
                            }
                        }

                        Item { Layout.fillWidth: true }

                        ColumnLayout {
                            Text {
                                text: "68.5%"
                                color: "white"
                                font.pixelSize: 18
                                font.bold: true
                            }
                            Text {
                                text: qsTr("胜率")
                                color: "rgba(255,255,255,0.7)"
                                font.pixelSize: 10
                            }
                        }

                        Item { Layout.fillWidth: true }

                        ColumnLayout {
                            Text {
                                text: "12"
                                color: "white"
                                font.pixelSize: 18
                                font.bold: true
                            }
                            Text {
                                text: qsTr("交易次数")
                                color: "rgba(255,255,255,0.7)"
                                font.pixelSize: 10
                            }
                        }
                    }
                }

                MouseArea {
                    anchors.fill: parent
                    onClicked: console.log("Clicked Moving Average Strategy")
                }
            }

            Rectangle {
                Layout.fillWidth: true
                Layout.leftMargin: 12
                Layout.rightMargin: 12
                height: 120
                radius: 10
                gradient: Gradient {
                    GradientStop { position: 0.0; color: "#6a1b9a" }
                    GradientStop { position: 1.0; color: "#ab47bc" }
                }

                ColumnLayout {
                    anchors.fill: parent
                    anchors.margins: 16

                    RowLayout {
                        Text {
                            text: qsTr("景气成长策略")
                            color: "white"
                            font.pixelSize: 16
                            font.bold: true
                        }
                        Item { Layout.fillWidth: true }
                        Rectangle {
                            width: 56
                            height: 24
                            radius: 12
                            color: "rgba(255,255,255,0.25)"

                            Text {
                                anchors.centerIn: parent
                                text: qsTr("已暂停")
                                color: "white"
                                font.pixelSize: 11
                            }
                        }
                    }

                    Text {
                        text: qsTr("MA60向上+缩量回调+业绩增长共振策略")
                        color: "rgba(255,255,255,0.8)"
                        font.pixelSize: 12
                        Layout.topMargin: 4
                    }

                    Item { Layout.fillHeight: true }

                    RowLayout {
                        Layout.fillWidth: true

                        ColumnLayout {
                            Text {
                                text: "23.4%"
                                color: "white"
                                font.pixelSize: 18
                                font.bold: true
                            }
                            Text {
                                text: qsTr("年化收益")
                                color: "rgba(255,255,255,0.7)"
                                font.pixelSize: 10
                            }
                        }

                        Item { Layout.fillWidth: true }

                        ColumnLayout {
                            Text {
                                text: "72.3%"
                                color: "white"
                                font.pixelSize: 18
                                font.bold: true
                            }
                            Text {
                                text: qsTr("胜率")
                                color: "rgba(255,255,255,0.7)"
                                font.pixelSize: 10
                            }
                        }

                        Item { Layout.fillWidth: true }

                        ColumnLayout {
                            Text {
                                text: "8"
                                color: "white"
                                font.pixelSize: 18
                                font.bold: true
                            }
                            Text {
                                text: qsTr("交易次数")
                                color: "rgba(255,255,255,0.7)"
                                font.pixelSize: 10
                            }
                        }
                    }
                }

                MouseArea {
                    anchors.fill: parent
                    onClicked: console.log("Clicked Prosperity Growth Strategy")
                }
            }

            Rectangle {
                Layout.fillWidth: true
                Layout.leftMargin: 12
                Layout.rightMargin: 12
                height: 56
                radius: 8
                color: "white"
                border.color: "#e0e0e0"
                border.width: 1

                RowLayout {
                    anchors.fill: parent
                    anchors.leftMargin: 12
                    anchors.rightMargin: 12

                    Rectangle {
                        width: 32
                        height: 32
                        radius: 16
                        color: "#e8f5e9"

                        Text {
                            anchors.centerIn: parent
                            text: "+"
                            color: "#4caf50"
                            font.pixelSize: 20
                            font.bold: true
                        }
                    }

                    Text {
                        text: qsTr("新建策略")
                        color: "#333"
                        font.pixelSize: 14
                        Layout.leftMargin: 12
                    }

                    Item { Layout.fillWidth: true }

                    Text {
                        text: ">"
                        color: "#ccc"
                        font.pixelSize: 16
                    }
                }

                MouseArea {
                    anchors.fill: parent
                    onClicked: console.log("New strategy")
                }
            }

            Item {
                Layout.fillWidth: true
                Layout.preferredHeight: 16
            }
        }
    }
}
