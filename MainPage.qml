import QtQuick 2.5
import QtQuick.Layouts 1.1
import Material 0.1 as Material

CustomPage {
    id: page
    
    title: qsTr("Gomoku Duel")
    
    actionBar {
        customContent: MouseArea {
            anchors.fill: parent
            
            property variant clickPos: "1,1"
            
            onPressed: {
                clickPos  = Qt.point(mouse.x,mouse.y)
            }
            
            onPositionChanged: {
                var delta = Qt.point(mouse.x-clickPos.x, mouse.y-clickPos.y)
                window.x += delta.x;
                window.y += delta.y;
            }
            
            Text {
                text: page.title
                anchors.centerIn: parent
                font.pixelSize: parent.height * 0.5
            }
        }
    }
    
//    anchors.fill: parent  // will result in no transition
    actionBar.hidden: false
    
    Image {
        id: title
        source: "qrc:/image/title.png"
        anchors.centerIn: parent
        scale: window.scale
        
        states: [
            State {
                name: "hidden"
                PropertyChanges { target: title; anchors.verticalCenterOffset: -window.width / 4 * 1.5 }
                PropertyChanges { target: title; opacity: 0.0 }
            },
            State {
                name: "shown"
                PropertyChanges { target: title; anchors.verticalCenterOffset: -window.width / 4 }
                PropertyChanges { target: title; opacity: 1.0 }
            }
        ]
        state: "hidden"
        transitions: Transition {
            PropertyAnimation {
                id: titleLoadAnimation
                properties: "opacity, anchors.verticalCenterOffset"
                duration: 1000; easing.type: Easing.OutQuad
            }
        }
    }
    
    ColumnLayout {
        id: buttons
        anchors {
            margins: window.width * 0.1
            left: parent.left
            right: parent.right
            bottom: parent.bottom
        }
        spacing: window.width * 0.05
        
        TextButton {
            id: networkDuelButton
            text: "Network Duel"
            opacity: 0.0; enabled: false
            fontSize: 60 * window.scale
            anchors.horizontalCenter: parent.horizontalCenter
            onClicked: pageStack.push("qrc:/ConnectPage.qml")
        }
        
        TextButton {
            id: profileButton
            text: "Profile"
            opacity: 0.0; enabled: false
            fontSize: 60 * window.scale
            anchors.horizontalCenter: parent.horizontalCenter
        }
        
        TextButton {
            id: quitButton
            text: "Quit"
            opacity: 0.0; enabled: false
            fontSize: 60 * window.scale
            anchors.horizontalCenter: parent.horizontalCenter
            onClicked: window.close()
        }
    }
    SequentialAnimation {
        id: buttonLoadAnimation
        
        property int buttonLoadTime: 400
        
        PauseAnimation { duration: titleLoadAnimation.duration * 0.85 }
        PropertyAnimation {
            target: networkDuelButton; properties: "opacity, scale"
            duration: buttonLoadAnimation.buttonLoadTime; from: 0.0; to: 1.0
        }
        PropertyAnimation {
            target: profileButton; properties: "opacity, scale"
            duration: buttonLoadAnimation.buttonLoadTime; from: 0.0; to: 1.0
        }
        PropertyAnimation {
            target: quitButton; properties: "opacity, scale"
            duration: buttonLoadAnimation.buttonLoadTime; from: 0.0; to: 1.0
        }
        ScriptAction {
            script: {
                networkDuelButton.enabled = true
                profileButton.enabled = true
                quitButton.enabled = true
            }
        }
    }
    
    Component.onCompleted: {
        title.state = "shown"
        buttonLoadAnimation.start()
    }
}
