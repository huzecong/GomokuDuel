import QtQuick 2.5
import QtQuick.Window 2.2
import QtQuick.Dialogs 1.2
import QtQuick.Layouts 1.1
import Material 0.1 as Material
import com.Kanari.GomokuDuel 1.0

Material.ApplicationWindow {
	
	id: window
	visible: true
	
	clientSideDecorations: true
	
	color: "black"
	
	title: qsTr("Gomoku Duel")
	
	property bool largeScreen: Screen.desktopAvailableHeight >= 800
	
	width: largeScreen ? 1200 : 920
	height: largeScreen? 900 : 690
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
		ParallelAnimation {
			PropertyAnimation {
				target: initialPage; property: "opacity"
				duration: 750; from: 1.0; to: 0.0
			}
			PropertyAnimation {
				target: bg; property: "opacity"
				duration: 900; from: 1.0; to: 0.0
			}
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
	
	initialPage: GamePage {
		Component.onCompleted: {
			Material.Units.multiplier = window.largeScreen ? 2.0 : 1.0
			console.log(Material.Units.multiplier)
		}
	}
}
