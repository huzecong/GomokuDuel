import QtQuick 2.5
import QtQuick.Layouts 1.1
import Material 0.1 as Material
import Material.ListItems 0.1 as ListItem
import Material.Extras 0.1 as Extras
import com.Kanari.GomokuDuel 1.0

CustomPage {
    id: page
    
    title: qsTr("Select Opponent")
    
    function loadGame(isHost, ip, p1Name, p1Avatar, p2Name, p2Avatar, myself) {
        console.log(isHost, ip, p1Name, p1Avatar, p2Name, p2Avatar, myself)
        GameStorage.isHost = isHost
        GameStorage.opponentIP = ip
        GameStorage.player1Name = p1Name
        GameStorage.player1AvatarId = p1Avatar
        GameStorage.player2Name = p2Name
        GameStorage.player2AvatarId = p2Avatar
        GameStorage.myself = myself
        manager.reset()
        pageStack.push("qrc:/GamePage.qml")
    }
    
    BroadcastManager {
        id: manager
        
        isHost: false
        profileName: GameStorage.profileName
        avatarId: GameStorage.avatarId
        
        onHostTimeout: {
            if (!connectToHostDialog.done
                    && connectToHostDialog.opponentIP == ip
                    && connectToHostDialog.profileName == name) {
                connectToHostDialog.done = true
                connectToHostDialog.timeoutDialog.show()
            }
        }
        onNewOpponent: {
            createMatchDialog.profileName = name
            createMatchDialog.opponentIP = ip
            createMatchDialog.avatarSource = avatarId
            createMatchDialog.state = "found"
        }
        onRequestAccepted: {
            if (!connectToHostDialog.done) {
                connectToHostDialog.done = true
                connectToHostDialog.close()
                manager.confirmJoinRequest(ip)
                
                loadGame(manager.isHost, connectToHostDialog.opponentIP,
                         connectToHostDialog.profileName,
                         connectToHostDialog.avatarSource,
                         GameStorage.profileName,
                         GameStorage.avatarId, 1)
            }
        }
        onRequestRefused: {
            if (!connectToHostDialog.done) {
                connectToHostDialog.done = true
                connectToHostDialog.refuseDialog.show()
            }
        }
        onConnectionConfirmed: {
            createMatchDialog.done = true
            createMatchDialog.close()
            
            loadGame(manager.isHost, createMatchDialog.opponentIP,
                     GameStorage.profileName,
                     GameStorage.avatarId,
                     createMatchDialog.profileName,
                     createMatchDialog.avatarSource, 0)
        }
    }
    
    CreateMatchDialog {
        id: createMatchDialog
        ipAddress: manager.refreshIP()
        
        onAbort: {
            createMatchDialog.done = true
            manager.abortHost()
            close()
        }
        onAccept: {
            manager.respondJoinRequest(opponentIP, true)
            positiveButton.enabled = false
            negativeButton.enabled = false
            done = false
            timer.start()
        }
        onRefuse: {
            manager.respondJoinRequest(opponentIP, false)
            createMatchDialog.state = "wait"
        }
    }
    ManualIPDialog {
        id: manualIPDialog
        
        onAccepted: {
            manager.connectToHost(ipAddress)
            connectToHostDialog.showDialog("", ipAddress, "")
        }
    }
    ConnectToHostDialog {
        id: connectToHostDialog
        ipAddress: manager.hostAddress
        
        onAbort: {
            connectToHostDialog.done = true
            manager.abortConnectToHost()
            close()
        }
    }
    
    Material.View {
        id: view
        
        anchors {
            left: parent.left
            right: parent.right
            margins: parent.width * 0.2
        }
        height: parent.height

        elevation: 10
        
        ListItem.Standard {
            id: createMatchItem
            text: "Create match..."
            onClicked: {
                manager.startHost()
                createMatchDialog.showDialog()
            }
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
                    
                    property int rounds: 10
                    property double winningRate: 50.0
                    
                    subText: qsTr("You've had %1 matches with this player,
with a winning rate of %2%.").arg(Number(rounds)).arg(Number(winningRate).toFixed(1))
                    valueText: "IP: %1".arg(model.ip)
                    
                    maximumLineCount: 3
                    
                    focused: ListView.isCurrentItem
                    onClicked: {
                        if (list.currentIndex == index) {
                            manager.connectToHost(model.ip)
                            console.log(model.ip)
                            connectToHostDialog.showDialog(model.name, model.ip, model.avatarId)
                        } else {
                            list.currentIndex = index
                        }
                    }
                    
                    secondaryItem: Material.Icon {
                        name: "navigation/chevron_right"
                        anchors.verticalCenter: parent.verticalCenter
                        size: list.currentIndex == index ? Material.Units.dp(32): 0
                        
                        Behavior on size {
                            NumberAnimation { duration: 100 }
                        }
                    }
                    
                    action: AnimatedImage {
                        source: "qrc:/avatar/%1".arg(model.avatarId)
                        anchors.centerIn: parent
                        fillMode: Image.PreserveAspectCrop
                        width: Material.Units.dp(40)
                        height: Material.Units.dp(40)
                    }
                }
            }
        }
        
        ListItem.Subheader {
            anchors.top: beforeSubheader.bottom
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
