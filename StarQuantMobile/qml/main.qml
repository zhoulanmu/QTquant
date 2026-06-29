import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import QtQuick.Window

ApplicationWindow {
    id: root
    visible: true
    width: 360
    height: 640
    minimumWidth: 320
    minimumHeight: 480
    title: qsTr("星策 StarQuant")

    color: "#f5f5f5"

    StackView {
        id: stackView
        anchors.fill: parent
        anchors.bottom: tabBar.top
        initialItem: homePage

        Component {
            id: homePage
            HomePage { }
        }

        Component {
            id: marketPage
            MarketPage { }
        }

        Component {
            id: strategyPage
            StrategyPage { }
        }

        Component {
            id: screenerPage
            ScreenerPage { }
        }

        Component {
            id: portfolioPage
            PortfolioPage { }
        }
    }

    TabBar {
        id: tabBar
        anchors.bottom: parent.bottom
        width: parent.width
        height: 56
        background: Rectangle {
            color: "white"
            border.top.color: "#e0e0e0"
            border.top.width: 1
        }

        TabButton {
            text: qsTr("首页")
            width: parent.width / 5
            icon.source: "qrc:/StarQuant/resources/starquant_stars.png"
            onClicked: stackView.replace(homePage)
        }

        TabButton {
            text: qsTr("行情")
            width: parent.width / 5
            onClicked: stackView.replace(marketPage)
        }

        TabButton {
            text: qsTr("选股")
            width: parent.width / 5
            onClicked: stackView.replace(screenerPage)
        }

        TabButton {
            text: qsTr("策略")
            width: parent.width / 5
            onClicked: stackView.replace(strategyPage)
        }

        TabButton {
            text: qsTr("资产")
            width: parent.width / 5
            onClicked: stackView.replace(portfolioPage)
        }
    }
}
