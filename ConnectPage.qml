import QtQuick 2.5
import QtQuick.Layouts 1.1
import Material 0.1 as Material
import Material.ListItems 0.1 as ListItem
import Material.Extras 0.1 as Extras

import com.Kanari.NetworkManager 1.0

CustomPage {
    id: page
    
    title: qsTr("Select Opponent")
    
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

    actionBar.hidden: false
    
    NetworkManager {
        id: manager
    }
    
    CreateMatchDialog {
        id: createMatchDialog
    }
    
    ManualIPDialog {
        id: manualIPDialog
    }
    
    Material.View {
        id: view
        
        anchors {
            left: parent.left
            right: parent.right
            margins: parent.width * 0.2
        }
        height: parent.height

        elevation: 1
        
        ListItem.Standard {
            id: createMatchItem
            text: "Create match..."
            onClicked: createMatchDialog.show()
        }
        
        ListItem.Subheader {
            id: beforeSubheader
            anchors.top: createMatchItem.bottom
            text: "Available matches"
        }
        
        Material.View {
            width: parent.width
            anchors.top: beforeSubheader.bottom
            anchors.bottom: manuallyConnectItem.top
            
            ListView {
                id: list
                anchors.fill: parent
                
                snapMode: ListView.SnapToItem
//                preferredHighlightBegin: 0
//                preferredHighlightEnd: height
//                highlightRangeMode: ListView.ApplyRange
//                keyNavigationWraps: true
                
                model: manager.hostList
                delegate: CustomSubtitled {
                    text: model.name
                    subText: qsTr("You've had %1 matches with this player,
with a winning rate of %2%.").arg(Number(model.rounds)).arg(Number(model.winningRate).toFixed(1))
                    valueText: "IP: %1".arg(model.ip)
                    
                    maximumLineCount: 3
                    
                    focused: ListView.isCurrentItem
                    onClicked: list.currentIndex = index
                    
                    secondaryItem: Material.Icon {
                        name: "navigation/chevron_right"
                        anchors.verticalCenter: parent.verticalCenter
                        size: list.currentIndex == index ? Material.Units.dp(32): 0
                        
                        Behavior on size {
                            NumberAnimation { duration: 100 }
                        }
                    }
                    
                    action: AnimatedImage {
                        source: "qrc:/image/avatar.gif"
                        anchors.centerIn: parent
                        fillMode: Image.PreserveAspectCrop
                        width: Material.Units.dp(40)
                        height: Material.Units.dp(40)
                    }
                }
            }
        }
        
        ListItem.Subheader {
            visible: list.count == 0
            text: "    Searching for existing matches..."
            style: "subheading"
            spacing: Material.Units.dp(10)
        }
        
        ListItem.Standard {
            id: manuallyConnectItem
            anchors.bottom: parent.bottom
            text: "Manually connect..."
            onClicked: manualIPDialog.show()
        }
    }
}
