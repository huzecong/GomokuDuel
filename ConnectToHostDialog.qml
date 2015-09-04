import QtQuick 2.0
import com.Kanari.GomokuDuel 1.0

CreateMatchDialog {
    id: root
    state: "found"
    
    property alias data: data
    
    NetworkHostData {
        id: data
    }
    
    function show(data) {
        avatarSource = data.avatarId;
        opponentIP = data.ip;
        profileName = data.name;
        rounds = data.rounds;
        winningRate = data.winningRate;
    }
}

