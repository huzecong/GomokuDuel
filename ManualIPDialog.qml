import QtQuick 2.5
import QtQuick.Controls 1.4
import QtQuick.Layouts 1.1
import Material 0.1

Dialog {
    id: root
    
    property alias ipAddress: ipText.text
    
    title: qsTr("Input opponent IP")
    
    dismissOnTap: true
    
    TextField {
        id: ipText
        text: ""
        font.family: "Consolas"
        font.pointSize: Units.dp(20)
//        inputMask: "000.000.000.000;"
        focus: true
        width: parent.width
        placeholderText: "Opponent IP"
        Component.onCompleted: ipText.cursorPosition = 0
    }
    
    GridLayout {
        columns: 5
        
        Repeater {
            model: 10
            delegate: ClickableSquare {
                text: index
                onClicked: {
                    ipText.insert(ipText.cursorPosition, String(index))
//                    ++ipText.cursorPosition
                }
            }
        }
        ClickableSquare {
            Layout.topMargin: Units.dp(10)
            text: ""
            Icon {
                name: "navigation/arrow_back"
                anchors.centerIn: parent
                size: Units.dp(25)
            }
            onClicked: {
                if (ipText.cursorPosition != 0) {
                    --ipText.cursorPosition
                }
            }
        }
        ClickableSquare {
            Layout.topMargin: Units.dp(10)
            text: ""
            Icon {
                name: "navigation/arrow_forward"
                anchors.centerIn: parent
                size: Units.dp(25)
            }
            onClicked: {
                if (ipText.cursorPosition != ipText.length) {
                    ++ipText.cursorPosition
                }
            }
        }
        ClickableSquare {
            Layout.topMargin: Units.dp(10)
            Layout.columnSpan: 2
            Layout.alignment: Qt.AlignRight
            text: "."
            onClicked: {
                ipText.insert(ipText.cursorPosition, ".")
//                ++ipText.cursorPosition
            }
        }
        ClickableSquare {
            Layout.topMargin: Units.dp(10)
            text: ""
            Icon {
                source: "qrc:/image/backspace.svg"
                anchors.centerIn: parent
                width: Units.dp(25)
                height: Units.dp(20)
            }
            onClicked: {
                if (ipText.cursorPosition != 0) {
                    ipText.remove(ipText.cursorPosition - 1, ipText.cursorPosition)
                }
            }
        }
    }
}
