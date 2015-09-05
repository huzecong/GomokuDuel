import QtQuick 2.5
import QtQuick.Layouts 1.1
import Material 0.1 as Material
import Material.ListItems 0.1 as ListItem
import com.Kanari.GomokuDuel 1.0

CustomPage {
    id: page
    
    title: GameStorage.myself == -1 ? "Duel between %1 and %2".arg(GameStorage.player1Name).arg(GameStorage.player2Name)
               : "Duel with %1".arg(GameStorage.myself == 1 ? GameStorage.player1Name : GameStorage.player2Name)
    canGoBack: false
    
    property bool gameStarted: false
    onGameStartedChanged: {
        logic.gameStarted = gameStarted
        if (gameStarted == false) {
            readyButton.enabled = true
            readyButton.readyClicked = false
        }
    }
    
    property alias logic: board.logic
    GameBoard {
        id: board
        
        width: height * (1029.0 / 1038.0)
        height: parent.height - Material.Units.dp(40)
        x: Material.Units.dp(20)
        y: Material.Units.dp(20)
        
        gameStarted: page.gameStarted
        
        onDropPiece: network.sendDropPiece(x, y, GameStorage.myself)
        onTimeout: {
            network.sendTimeout(player)
            logic.nextTurn()
        }
        onGameEnd: {
            page.gameStarted = false
            if (result == -1) {
                gameEndBanner.source = "qrc:/image/draw.png"
            } else if (result == GameStorage.myself) {
                gameEndBanner.source = "qrc:/image/win.png"
            } else {
                gameEndBanner.source = "qrc:/image/lose.png"
            }
            gameEndBanner.state = "shown"
            network.gameEnd(result)
        }
        
        Image {
            id: gameEndBanner
            anchors.centerIn: parent
            
            property double scale: board.width / 600
            width: sourceSize.width * scale
            height: sourceSize.height * scale
            
            states: [
                State {
                    name: "hidden"
                    PropertyChanges {
                        target: gameEndBanner
                        opacity: 0.0
                        anchors.verticalCenterOffset: -board.height * 0.2
                    }
                },
                State {
                    name: "shown"
                    PropertyChanges {
                        target: gameEndBanner
                        opacity: 1.0
                        anchors.verticalCenterOffset: 0.0
                    }
                }
            ]
            state: "hidden"
            transitions: [
                Transition {
                    from: "hidden"; to: "shown"
                    SequentialAnimation {
                        PropertyAnimation {
                            properties: "anchors.verticalCenterOffset, opacity"
                            duration: 1500
                            easing.type: Easing.OutQuad
                        }
                        PauseAnimation { duration: 1000 }
                        ScriptAction { script: gameEndBanner.state = "hidden" }
                    }
                },
                Transition {
                    from: "shown"; to: "hidden"
                    SequentialAnimation {
                        PropertyAnimation {
                            properties: "anchors.verticalCenterOffset, opacity"
                            duration: 1000
                        }
                    }
                }
            ]
        }
    }
    
    Timer {
        id: timer
        
    }
    
    Material.Dialog {
        id: errorDialog
        title: qsTr("Connection error")
        text: qsTr("Your opponent may have gone offline")
        negativeButtonText: qsTr("Done")
        positiveButton.visible: false
        dismissOnTap: false
        
        onRejected: {
            pageStack.pop()
            pageStack.pop()
        }
    }
    Material.Dialog {
        id: requestDialog
        title: qsTr("Confirm request")
        negativeButtonText: qsTr("Refuse")
        positiveButtonText: qsTr("Accept")
        dismissOnTap: false
        
        onAccepted: {
            network.sendResponse("accept", GameStorage.myself)
        }
        onRejected: {
            network.sendResponse("refuse", GameStorage.myself)
            logic.startTimer()
        }
    }
    Material.Dialog {
        id: waitDialog
        hasActions: false
        dismissOnTap: false
        
        function showDialog(request) {
            logic.pauseTimer()
            title = qsTr("Request for " + request + " sent")
            show()
        }
        onClosed: logic.startTimer()
        
        RowLayout {
            id: waitContent
            spacing: Material.Units.dp(20)
            
            Material.ProgressCircle {
                color: Material.Palette.colors["blue"]["500"]
                indeterminate: true
                width: Material.Units.dp(50)
                height: Material.Units.dp(50)
                dashThickness: Material.Units.dp(5)
            }
            Material.Label {
                id: waitLabel
                text: qsTr("Waiting for response from opponent...")
                style: "dialog"
                Layout.alignment: Qt.AlignVCenter
            }
        }
    }
    
    function delay(delayTime, cb) {
        var delayTimer = Qt.createQmlObject('import QtQuick 2.0; Timer { }', page);
        delayTimer.interval = delayTime;
        delayTimer.repeat = false;
        delayTimer.triggered.connect(cb);
        delayTimer.start();
    }
    
    NetworkManager {
        id: network
        myself: GameStorage.myself
        
        Component.onCompleted: {
            if (GameStorage.isHost == true) {
                network.init(1, GameStorage.opponentIP, GameStorage.player1Name, GameStorage.player2Name, 0)
            } else if (GameStorage.isHost == false) {
                network.init(0, GameStorage.opponentIP, GameStorage.player1Name, GameStorage.player2Name, 0)
            }
        }
        
        onCriticalError: {
            if (!errorDialog.showing) {
                errorDialog.text = message
                errorDialog.show()
            }
        }
        onRequest: {
            var text = qsTr("Your opponent wants to ")
            if (request == "exit") {
                text = text.concat(qsTr("quit this match"))
            } else if (request == "undo") {
                text = text.concat(qsTr("undo last move"))
            } else if (request == "surrender") {
                text = text.concat(qsTr("declare defeat"))
            } else if (request == "draw") {
                text = text.concat(qsTr("declare a draw"))
            }
            requestDialog.text = text
            requestDialog.show()
        }
        onResponse: waitDialog.close()
        onStartGame: {
            gameStarted = true
            logic.newGame()
        }
        onUndo: logic.undo(player)
        onSurrender: logic.surrender(player)
        onExit: {
            delay(200, pageStack.pop)
        }
        onDraw: logic.draw()
        onTimeout: logic.nextTurn()
        onShowPiece: logic.dropPiece(x, y)
    }
    
    Material.View {
        id: view
        
        anchors {
            left: board.right
            right: parent.right
            rightMargin: Material.Units.dp(20)
            leftMargin: Material.Units.dp(40)
        }
        height: parent.height

        elevation: 10
        
        ColumnLayout {
            
            width: parent.width
            height: parent.height
            
            ListItem.Subheader {
                text: "Match Summary"
            }
            
            RowLayout {
                id: player1tItem
                    
                Text {
                    id: player1ScoreText
                    Layout.alignment: Qt.AlignVCenter | Qt.AlignLeft
                    Layout.leftMargin: Material.Units.dp(15)
                    Layout.rightMargin: Material.Units.dp(0)
                    font.family: "Bank Gothic"
                    font.pixelSize: Material.Units.dp(30)
                    color: Material.Theme.light.textColor
                    text: board.p1Score
//                    width: Material.Units.dp(30)
                }
                CustomSubtitled {
                    Layout.alignment: Qt.AlignVCenter | Qt.AlignRight
                    Layout.fillWidth: true
                    anchors.left: text.right
                    
                    text: GameStorage.player1Name
                    subText: "Total time:\n" + logic.p1Time
                    maximumLineCount: 3
                    focused: board.currentPlayer === 0
                    interactive: false
                    
                    secondaryItem: ColumnLayout {
                        anchors.centerIn: parent
                        Material.Label {
                            Layout.alignment: Qt.AlignHCenter
                            style: "body1"
                            text: "Remain"
                        }
                        Text {
                            id: player1TimeText
                            Layout.alignment: Qt.AlignHCenter
                            font.family: "DFNewChuan-B5"
                            font.pixelSize: Material.Units.dp(25)
                            color: Material.Theme.light.textColor
                            text: board.currentPlayer === 0 ? logic.remainTime : logic.turnTime
                        }
                    }
                    
                    action: AnimatedImage {
                        Layout.alignment: Qt.AlignVCenter | Qt.AlignLeft
                        width: Material.Units.dp(40)
                        height: Material.Units.dp(40)
                        Layout.fillHeight: true
                        source: "qrc:/avatar/%1".arg(GameStorage.player1AvatarId)
                        fillMode: Image.PreserveAspectCrop
                    }
                }
            }
            
            RowLayout {
                id: player2Item
                    
                Text {
                    id: player2ScoreText
                    Layout.alignment: Qt.AlignVCenter | Qt.AlignLeft
                    Layout.leftMargin: Material.Units.dp(15)
                    Layout.rightMargin: Material.Units.dp(0)
                    font.family: "Bank Gothic"
                    font.pixelSize: Material.Units.dp(30)
                    color: Material.Theme.light.textColor
                    text: board.p2Score
//                    width: Material.Units.dp(30)
                }
                CustomSubtitled {
                    Layout.alignment: Qt.AlignVCenter | Qt.AlignRight
                    Layout.fillWidth: true
                    anchors.left: text.right
                    
                    text: GameStorage.player2Name
                    subText: "Total time:\n" + logic.p2Time
                    maximumLineCount: 3
                    focused: board.currentPlayer === 1
                    interactive: false
                    
                    secondaryItem: ColumnLayout {
                        anchors.centerIn: parent
                        Material.Label {
                            Layout.alignment: Qt.AlignHCenter
                            style: "body1"
                            text: "Remain"
                        }
                        Text {
                            id: player2TimeText
                            Layout.alignment: Qt.AlignHCenter
                            font.family: "DFNewChuan-B5"
                            font.pixelSize: Material.Units.dp(25)
                            color: Material.Theme.light.textColor
                            text: board.currentPlayer === 1 ? logic.remainTime : logic.turnTime
                        }
                    }
                    
                    action: AnimatedImage {
                        Layout.alignment: Qt.AlignVCenter | Qt.AlignLeft
                        width: Material.Units.dp(40)
                        height: Material.Units.dp(40)
                        Layout.fillHeight: true
                        source: "qrc:/avatar/%1".arg(GameStorage.player2AvatarId)
                        fillMode: Image.PreserveAspectCrop
                    }
                }
            }
            
            
            ListItem.Subheader {
                text: "Chat"
            }
            
            Material.View {
                id: chatBox
                elevation: 1
                elevationInverted: true
                Layout.fillWidth: true
                Layout.fillHeight: true
                Layout.leftMargin: Material.Units.dp(10)
                Layout.rightMargin: Material.Units.dp(10)
                Layout.bottomMargin: Material.Units.dp(5)
                
                Flickable {
                    id: flick
                    anchors.fill: parent
                    contentWidth: edit.paintedWidth
                    contentHeight: edit.paintedHeight
                    clip: true
                    
                    function ensureVisible(r) {
                        if (contentX >= r.x)
                            contentX = r.x;
                        else if (contentX + width <= r.x + r.width)
                            contentX = r.x + r.width - width;
                        if (contentY >= r.y)
                            contentY = r.y;
                        else if (contentY + height <= r.y + r.height)
                            contentY = r.y + r.height - height;
                    }
                    
                    TextEdit {
                        id: edit
                        textFormat: TextEdit.RichText
                        textMargin: Material.Units.dp(5)
                        wrapMode: TextEdit.Wrap
                        readOnly: true
                        font.pixelSize: Material.Units.dp(14)
                        
                        text: network.chatMessage
                        width: flick.width
                        height: flick.height
                        focus: true
                        onTextChanged: {
                            selectAll()
                            cursorPosition = selectionEnd
                        }
                        onCursorRectangleChanged: flick.ensureVisible(cursorRectangle)
                    }
                }
            }
            
            RowLayout {
                Layout.bottomMargin: Material.Units.dp(10)
                Layout.alignment: Qt.AlignHCenter | Qt.AlignBottom
                
                Material.TextField {
                    id: chatTextField
                    placeholderText: "Chat here..."
                    Keys.onEnterPressed: {
                        if (chatTextField.text != "") {
                            network.sendChat(chatTextField.text, GameStorage.myself)
                            chatTextField.text = ""
                        }
                    }
                }
                Material.Button {
                    id: chatSendButton
                    elevation: 1
                    text: "Send"
                    context: "dialog"
                    onClicked: {
                        if (chatTextField.text != "") {
                            network.sendChat(chatTextField.text, GameStorage.myself)
                            chatTextField.text = ""
                        }
                        chatTextField.forceActiveFocus()
                    }
                }
            }
            
            ListItem.Subheader {
                Layout.alignment: Qt.AlignBottom
                text: "Game Actions"
            }
            
            GridLayout {
                Layout.alignment: Qt.AlignHCenter | Qt.AlignBottom
                Layout.bottomMargin: Material.Units.dp(20)
                columns: 2
                columnSpacing: Material.Units.dp(20)
                rowSpacing: Material.Units.dp(20)
                Material.Button {
                    id: readyButton
                    text: "Ready"
                    enabled: true
                    elevation: 1
                    property bool readyClicked: false
                    onClicked: {
                        if (readyClicked == false) {
                            readyClicked = true
                            enabled = false
                            network.sendReady(GameStorage.myself)
                        }
                    }
                }
                Material.Button {
                    id: undoButton
                    text: "Undo"
                    enabled: gameStarted && logic.canUndo
                    elevation: 1
                    onClicked: {
                        network.sendRequest("undo", GameStorage.myself)
                        waitDialog.showDialog("undo")
                    }
                }
                Material.Button {
                    id: drawButton
                    text: "Draw"
                    enabled: gameStarted
                    elevation: 1
                    onClicked: {
                        network.sendRequest("draw", GameStorage.myself)
                        waitDialog.showDialog("draw")
                    }
                }
                Material.Button {
                    id: surrenderButton
                    text: "Give Up"
                    enabled: gameStarted
                    elevation: 1
                    onClicked: {
                        network.sendRequest("surrender", GameStorage.myself)
                        waitDialog.showDialog("defeat")
                    }
                }
                Material.Button {
                    id: saveLoadButton
                    text: "Save"
                    enabled: false
                    elevation: 1
                    onClicked: {
                        //network.sendRequest("exit", GameStorage.myself)
                        //waitDialog.showDialog("exit")
                    }
                }
                Material.Button {
                    id: exitButton
                    text: "Exit"
                    enabled: true
                    elevation: 1
                    onClicked: {
                        network.sendRequest("exit", GameStorage.myself)
                        waitDialog.showDialog("exit")
                    }
                }
            }
        }
    }
}

