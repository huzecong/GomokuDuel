import QtQuick 2.5
import QtQuick.Layouts 1.1
import Material 0.1 as Material
import Material.ListItems 0.1 as ListItem
import Material.Extras 0.1 as Extras
import com.Kanari.GomokuDuel 1.0

CustomPage {
    id: page
    
    title: qsTr("Profile Settings")
    
    property var avatars: ["gomoku.jpg", "gumi.jpg", "ha.gif", "honoka.jpg",
        "monkey.gif", "nanami.jpg", "rabbit.jpg", "sillyb.jpg", "think.gif",
        "xsk.gif", "young.jpg"]
    
    Material.View {
        id: view
        
        anchors {
            left: parent.left
            right: parent.right
            margins: parent.width * 0.2
        }
        height: parent.height

        elevation: 10
        
        ColumnLayout {
            anchors.fill: parent
            anchors.margins: Material.Units.dp(10)
            
            ListItem.Subheader {
                id: nameHeader
                text: "Profile Name"
            }
            
            Material.TextField {
                id: nameField
                Layout.fillWidth: true
                Layout.leftMargin: Material.Units.dp(20)
                Layout.rightMargin: Material.Units.dp(20)
                
                text: GameStorage.profileName
                placeholderText: "Your name"
                onTextChanged: {
                    GameStorage.setProfileName(text)
                }
            }
            
            ListItem.Subheader {
                id: avatarHeader
                text: "Profile Avatar"
            }
            
            GridLayout {
                id: gridView
                Layout.alignment: Qt.AlignHCenter | Qt.AlignTop
                columns: largeScreen ? 6 : 4
                columnSpacing: Material.Units.dp(10)
                rowSpacing: Material.Units.dp(10)
                
                Repeater {
                    model: avatars
                    delegate: ClickableSquare {
                        selected: GameStorage.avatarId === modelData
                        onClicked: GameStorage.avatarId = modelData
                        length: Material.Units.dp(80)
                        
                        AnimatedImage {
                            id: image
                            source: "qrc:/avatar/%1".arg(modelData)
                            anchors.centerIn: parent
                            fillMode: Image.PreserveAspectCrop
                            property double scale: Material.Units.dp(75) / Math.max(sourceSize.width, sourceSize.height)
                            width: sourceSize.width * scale
                            height: sourceSize.height* scale
                        }
                    }
                }
            }
        }
    }
}
