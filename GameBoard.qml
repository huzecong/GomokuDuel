import QtQuick 2.5
import QtQuick.Layouts 1.1
import Material 0.1 as Material
import Material.ListItems 0.1 as ListItem
import com.Kanari.GomokuDuel 1.0
import "GameBoardToggle.js" as Toggle

Item {
    id: root
    
    Image {
        source: "qrc:/image/board.png"
        anchors.fill: parent
    }
    
    property double scale: height / 780
    property double gridSize: 49.315 * scale
    property double upper: 36.8 * scale
    
    property alias p1Score: logic.p1Score
    property alias p2Score: logic.p2Score
    property alias currentPlayer: logic.currentPlayer
    property alias logic: logic
    
    signal dropPiece(int x, int y)
    signal gameEnd(int result)
    signal timeout(int player)
    
    property alias gameStarted: logic.gameStarted
    
    GameLogic {
        id: logic
        
        myself: GameStorage.myself
        Component.onCompleted: logic.init(0)
        
        onShowPiece: Toggle.showPiece(x, y, color)
        onRemovePiece: Toggle.removePiece(x, y)
        onResetAll: Toggle.resetAll()
        onGameEnd: root.gameEnd(result)
        onTimeout: root.timeout(player)
    }
    
    MouseArea {
        id: mouseArea
        anchors.fill: parent
        
        hoverEnabled: true
        
        Material.View {
            id: focusMark
            width: gridSize * 0.5
            height: width
            opacity: 0.0
            
            property int posX: 0
            property int posY: 0
            x: posX * gridSize + upper - width / 2
            y: posY * gridSize + upper - height / 2 + 3 * scale
            
            Behavior on x {
                NumberAnimation { duration: 50; easing.type: Easing.InOutQuad }
            }
            Behavior on y {
                NumberAnimation { duration: 50; easing.type: Easing.InOutQuad }
            }
            Behavior on opacity {
                NumberAnimation { duration: 50; easing.type: Easing.InOutQuad }
            }
            
            Image {
                source: "qrc:/image/focus.png"
                anchors.fill: parent
            }
        }
        
        onClicked: {
            if (GameStorage.myself == currentPlayer && logic.gameStarted) {
                var x = Math.floor((mouse.x - upper + gridSize / 2) / gridSize)
                var y = Math.floor((mouse.y - upper + gridSize / 2) / gridSize)
                console.log(x, y)
                if (logic.canDrop(x, y)) {
                    root.dropPiece(x, y)
                    logic.dropPiece(x, y)
                }
            }
        }
        onPositionChanged: {
            var x = Math.floor((mouse.x - upper + gridSize / 2) / gridSize)
            var y = Math.floor((mouse.y - upper + gridSize / 2) / gridSize)
            if (x >= 0 && x < 15) focusMark.posX = x;
            if (y >= 0 && y < 15) focusMark.posY = y;
        }
        onEntered: focusMark.opacity = 1.0;
        onExited: focusMark.opacity = 0.0;
    }
}

