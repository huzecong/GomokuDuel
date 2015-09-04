import QtQuick 2.5
import QtQuick.Window 2.2
import QtQuick.Dialogs 1.2
import QtQuick.Layouts 1.1
import Material 0.1 as Material
//import Material 0.1

Material.ApplicationWindow {
    
    id: window
    visible: true
    
    clientSideDecorations: true
    
    color: "black"
    
    title: qsTr("Gomoku Duel")
    
    width: Screen.desktopAvailableHeight >= 900 ? 1200 : 800
    height: Screen.desktopAvailableHeight >= 900 ? 900 : 600
/*    onHeightChanged: {
        width = Math.floor(height * 4 / 3)
    }
    onWidthChanged: {
        height = Math.floor(width * 3 / 4)
    }
    */
    maximumHeight: minimumHeight
    maximumWidth: minimumWidth
    property double scale: width / 1600
    
    onClosing: {
        close.accepted = false
        confirmQuitDialog.show()
    }
    
    Material.Dialog {
        id: confirmQuitDialog
        
        title: qsTr("Confirm quit", "quit dialog")
        text: qsTr("Are you sure you want to quit?")
        
        positiveButtonText: "Yes"
        negativeButtonText: "No"
        
        onAccepted: quitAnimation.start()
    }
    
    SequentialAnimation {
        id: quitAnimation
        running: false
        PauseAnimation {
            duration: 200
        }
        PropertyAnimation {
            target: initialPage; property: "opacity"
            duration: 750; from: 1.0; to: 0.0
        }
        ScriptAction {
            script: Qt.quit()
        }
    }
    
    Image {
        id: bg
        z: -1
        source: "qrc:/image/background.png"
        anchors.fill: parent
        fillMode: Image.PreserveAspectFit
    }
    
    initialPage: MainPage {
        Component.onCompleted: {
            Material.Units.multiplier = Screen.desktopAvailableHeight >= 900 ? 2.0 : 1.4
            console.log(Material.Units.multiplier)
        }
    }
}

