import QtQuick 2.5
import QtQuick.Layouts 1.1
import Material 0.1
import com.Kanari.GomokuDuel 1.0

CustomDialog {
	id: root
	
	property bool done: true
	
	Timer {
		id: timer
		repeat: false
		interval: 2500
		onTriggered: {
			root.positiveButton.enabled = true
			root.negativeButton.enabled = true
			if (!root.done) {
				root.done = true
				timeoutDialog.show()
			}
		}
	}
	
	property alias timeoutDialog: timeoutDialog
	property alias timer: timer
	
	Dialog {
		id: timeoutDialog
		title: qsTr("Connection timeout")
		text: qsTr("Your opponent may have gone offline")
		negativeButtonText: qsTr("Done")
		positiveButton.visible: false
		
		onRejected: {
			if (root.state == "connect") {
				root.close()
			} else {
				root.done = false
				root.state = "wait"
			}
		}
	}
	
	function showDialog() {
		done = false;
		positiveButton.enabled = true
		negativeButton.enabled = true
		show();
	}
	
	property string ipAddress
	
	property string avatarId
	property alias profileName: nameLabel.text
	property int uniqueId
	property string opponentIP: "0.1.2.3"
	property int rounds: GameStorage.rounds(uniqueId)
	property double winningRate: GameStorage.winningRate(uniqueId)
	
	property alias waitText: waitLabel.text
	property alias infoText: playerInfoLabel.text
	
	title: qsTr("Your IP address is: %1".arg(ipAddress))
	
	signal abort()
	signal accept()
	signal refuse()
	
	onRejected: {
		if (state == "found") {
			root.refuse()
		} else {
			root.abort()
		}
	}
	
	onAccepted: root.accept()
	
	states: [
		State {
			name: "wait"
			PropertyChanges {
				target: root
				negativeButtonText: qsTr("Abort")
				positiveButton.visible: false
			}
			PropertyChanges { target: waitContent; visible: true }
			PropertyChanges { target: foundContent; visible: false }
		},
		State {
			name: "found"
			PropertyChanges {
				target: root
				negativeButtonText: qsTr("Refuse")
				positiveButtonText: qsTr("Accept")
				positiveButton.visible: true
			}
			PropertyChanges { target: waitContent; visible: false }
			PropertyChanges { target: foundContent; visible: true }
		},
		State {
			name: "connect"
			PropertyChanges {
				target: root
				negativeButtonText: qsTr("Abort")
				positiveButton.visible: false
				
				onRejected: root.abort()
			}
			PropertyChanges { target: waitContent; visible: true }
			PropertyChanges { target: foundContent; visible: true }
		}
	]
	state: "wait"
	
	dismissOnTap: false
	
	RowLayout {
		id: waitContent
		spacing: Units.dp(20)
		visible: false
		
		ProgressCircle {
			color: Palette.colors["blue"]["500"]
			indeterminate: true
			width: Units.dp(50)
			height: Units.dp(50)
			dashThickness: Units.dp(5)
		}
		Label {
			id: waitLabel
			text: qsTr("Waiting for opponents...")
			style: "dialog"
			Layout.alignment: Qt.AlignVCenter
		}
	}
	
	ColumnLayout {
		id: foundContent
		width: opponentInfo.width// + Units.dp(10)
		spacing: Units.dp(10)
		visible: false
		
		Label {
			id: playerInfoLabel
			text: qsTr("A new opponent!")
		}
		
		Rectangle{
			
			Layout.alignment: Qt.AlignHCenter
			
			id: opponentInfo
			
			color: Palette.colors["grey"]["100"]
			width: layout.implicitWidth
			height: layout.implicitHeight
			
			Layout.fillWidth: true
			
			RowLayout {
				id: layout
				
				anchors.left: parent.left
				anchors.right: parent.right
				anchors.margins: Units.dp(16)
				
				spacing: Units.dp(16)
				
				Item {
					id: actionItem
					
					Layout.preferredWidth: Units.dp(40)
					Layout.preferredHeight: width
					Layout.alignment: Qt.AlignCenter
					Layout.column: 1
					
					AnimatedImage {
						id: avatar
						source: "qrc:/avatar/%1".arg(avatarId)
						anchors.centerIn: parent
						fillMode: Image.PreserveAspectCrop
						width: Units.dp(40)
						height: Units.dp(40)
					}
				}
				
				ColumnLayout {
					Layout.alignment: Qt.AlignLeft | Qt.AlignVCenter
					Layout.fillWidth: true
					Layout.column: 2
					
					spacing: Units.dp(3)
					
					RowLayout {
						Layout.fillWidth: true
						
						spacing: Units.dp(8)
						
						Label {
							id: nameLabel
							Layout.alignment: Qt.AlignVCenter
							Layout.fillWidth: true
							
							elide: Text.ElideRight
							style: "subheading"
							
							text: "Name"
						}
						
						Label {
							id: ipLabel
							
							Layout.alignment: Qt.AlignVCenter | Qt.AlignRight
							Layout.preferredWidth: implicitWidth
							
							color: Theme.light.subTextColor
							elide: Text.ElideRight
							horizontalAlignment: Qt.AlignHCenter
							style: "body1"
							
							text: "IP: %1".arg(root.opponentIP)
						}
					}
					
					Label {
						id: roundsLabel
						
						Layout.fillWidth: true
						Layout.preferredHeight: implicitHeight * 3 / 2
						
						color: Theme.light.subTextColor
						elide: Text.ElideRight
						wrapMode: Text.WordWrap
						style: "body1"
						
						maximumLineCount: 2
						
						text: "You've had %1 matches with this player,
with a winning rate of %2%.".arg(root.rounds).arg(Number(root.winningRate).toFixed(1))
					}
				}
			}
		}
	}
}
