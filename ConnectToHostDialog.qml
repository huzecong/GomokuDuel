import QtQuick 2.5
import QtQuick.Layouts 1.1
import Material 0.1
import com.Kanari.GomokuDuel 1.0

CreateMatchDialog {
    id: root
    
    state: "connect"
    
    waitText: qsTr("Connecting to host...")
    infoText: qsTr("Your opponent:")
    
    property alias refuseDialog: refuseDialog
    
    Dialog {
        id: refuseDialog
        title: qsTr("Match refused")
        text: qsTr("Your opponent refused this match")
        negativeButtonText: qsTr("Done")
        positiveButton.visible: false
        
        onRejected: {
            root.close()
        }
    }
    
    function showDialog(_name, _ip, _avatarId) {
        done = false
        profileName = _name
        opponentIP = _ip
        avatarSource = _avatarId
        show()
    }
}
