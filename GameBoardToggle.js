var N = 15
var piece = new Array(N * N);
var component = Qt.createComponent("BoardCircle.qml")

function index(x, y) {
	return x * N + y
}

function showPiece(x, y, color) {
    console.log(color)
    if (x < 0 || y < 0 || x >= N || y >= N) return false;
    
    var id = index(x, y)
    if (piece[id]) {
    } else {
        piece[id] = component.createObject(mouseArea)
        piece[id].posX = x
        piece[id].posY = y
    }
    
    piece[id].color = color
    piece[id].state = "shown"
    return true
}

function removePiece(x, y) {
    if (x < 0 || y < 0 || x >= N || y >= N) return ;
    
    var id = index(x, y)
    if (piece[id]) {
        piece[id].state = "hidden"
    }
}

function resetAll() {
    for (var i = 0; i < N; ++i)
        for (var j = 0; j < N; ++j) {
            var id = index(i, j)
            if (piece[id]) {
                piece[id].state = "hidden"
                piece[id] = null
            }
        }
}
