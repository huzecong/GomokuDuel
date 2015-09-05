import QtQuick 2.5
import Material 0.1

Item {
	id: root
    
	property double length: parent.parent.gridSize
	property string color: "black"
	property int posX: 0
	property int posY: 0
	x: posX * length + parent.parent.upper - width / 2
	y: posY * length + parent.parent.upper - height / 2
	
	states: [
		State {
			name: "hidden"
            PropertyChanges { target: root; width: length * 1.7; opacity: 0.0 }
		},
		State {
			name: "shown"
            PropertyChanges { target: root; width: length; opacity: 1.0 }
		}
	]
	state: "hidden"
    transitions: [
        Transition {
            from: "hidden"; to: "shown"
            PropertyAnimation { properties: "opacity, width"; duration: 200; easing.type: Easing.OutQuart }
        },
        Transition {
            from: "shown"; to: "hidden"
            PropertyAnimation { properties: "opacity, width"; duration: 200; easing.type: Easing.InQuart }
        }
    ]
	
	width: length
	height: width

	View {
		id: view
		anchors.centerIn: parent
		elevation: 2
		width: parent.width * 0.9
		height: width
		radius: width / 2
		antialiasing: true
		
		Image {
			id: rect
            source: "qrc:/image/%1.png".arg(root.color)
			width: parent.width
			height: width
			antialiasing: true
		}
	}
}

