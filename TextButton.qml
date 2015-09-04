import QtQuick 2.5
import QtGraphicalEffects 1.0

Item {
    id: root
    
    property alias text: text.text
    property alias font: text.font.family
    property double fontSize: 20
    property alias textColor: text.color
    property alias glowColor: glow.color
    property alias scale: text.scale
    property alias enabled: mouseArea.enabled
    
    signal clicked()
    
    width: text.implicitWidth
    height: text.implicitHeight
    
    Text {
        id: text
        text: "TextButton"
        font.family: "Hobo Std"
        font.pointSize: root.fontSize * scale
        property double scale: 1.0
        anchors.centerIn: parent
        
        Behavior on scale {
            NumberAnimation { duration: 100 }
        }
        
        MouseArea {
            id: mouseArea
            anchors.fill: parent
            onClicked: root.clicked()
            hoverEnabled: true
            onEntered: {
                glow.radius = 12
                text.scale = 1.1
            }
            onExited: {
                glow.radius = 0
                text.scale = 1.0
            }
        }
    }
    
    Glow {
        id: glow
        anchors.fill: text
        samples: 16
        color: "white"
        source: text
        
        Behavior on radius {
            NumberAnimation { duration: 100 }
        }
    }
}
