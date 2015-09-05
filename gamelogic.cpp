#include "gamelogic.h"

GameLogic::GameLogic(QObject *parent) : QObject(parent) {
	this->m_first = 1;
	this->m_color = None;
	emit currentPlayerChanged(currentPlayer());
	this->m_updateTimer.setInterval(200);
	this->m_timer.setInterval(TURN_TIME * 1000);
	this->m_timer.setSingleShot(true);
	QObject::connect(&this->m_timer, &QTimer::timeout, [this]() {
		emit timeout(currentPlayer());
	});
	QObject::connect(&this->m_updateTimer, &QTimer::timeout, this, &GameLogic::updateTime);
}

const QString GameLogic::colors[2] = {"black", "white"};

void GameLogic::init(int first) {
	this->m_first = first ^ 1;	// invert at game start
	for (int i = 0; i < 2; ++i)
		this->m_score[i] = this->m_time[i] = 0;
	emit currentPlayerChanged(currentPlayer());
	updateTime();
	emit p1ScoreChanged(p1Score());
	emit p2ScoreChanged(p2Score());
	emit canUndoChanged(canUndo());
}

void GameLogic::newGame() {
	qDebug() << "resetAll";
	emit resetAll();
	for (int i = 0; i < N; ++i)
		for (int j = 0; j < N; ++j)
			this->m_board[i][j] = None;
	this->m_first ^= 1;
	this->m_color = Black;
	this->m_steps = 0;
	emit canUndoChanged(canUndo());
	for (int i = 0; i < 2; ++i)
		this->m_time[i] = 0;
	updateTime();
	emit currentColorChanged(currentColor());
	emit currentPlayerChanged(currentPlayer());
	startTimer();
}

bool GameLogic::canDrop(int x, int y) const {
	return this->m_board[x][y] == None;
}

void GameLogic::dropPiece(int x, int y) {
	if (!canDrop(x, y)) return ;
	this->m_board[x][y] = this->m_color;
	this->m_history[++this->m_steps] = index(x, y);
	emit showPiece(x, y, currentColor());
	emit canUndoChanged(canUndo());
	int result = checkWin(x, y);
	if (result == 1) {
		++this->m_score[currentPlayer()];
		emit p1ScoreChanged(p1Score());
		emit p2ScoreChanged(p2Score());
		emit gameEnd(currentPlayer());
		this->m_color = -currentPlayer() - 1;
		emit currentPlayerChanged(currentPlayer());
	} else if (result == 0) {
		nextTurn();
	} else {
		draw();
	}
}

void GameLogic::nextTurn() {
	stopTimer();
	this->m_color ^= 1;
	emit currentColorChanged(currentColor());
	emit currentPlayerChanged(currentPlayer());
	startTimer();
}

void GameLogic::undo(int player) {
	stopTimer();
	int x, y;
	while (this->m_steps > 0) {
		x = this->m_history[this->m_steps] / N;
		y = this->m_history[this->m_steps] % N;
		--this->m_steps;
		emit removePiece(x, y);
		if (this->m_board[x][y] == player) break;
		this->m_board[x][y] = None;
	}
	this->m_board[x][y] = None;
	emit canUndoChanged(canUndo());
	if (currentPlayer() != player) nextTurn();
	else startTimer();
}

bool GameLogic::canUndo() const {
	if (this->m_steps >= 1
	    && colorToPlayer(colorAt(this->m_history[this->m_steps])) == this->m_myself)
		return true;
	if (this->m_steps >= 2
	    && colorToPlayer(colorAt(this->m_history[this->m_steps])) != this->m_myself
	    && colorToPlayer(colorAt(this->m_history[this->m_steps - 1])) == this->m_myself)
		return true;
	return false;
}

void GameLogic::surrender(int player) {
	++this->m_score[player ^ 1];
	emit p1ScoreChanged(p1Score());
	emit p2ScoreChanged(p2Score());
	emit gameEnd(player ^ 1);
	stopTimer();
	this->m_color = -currentPlayer() - 1;
	emit currentPlayerChanged(currentPlayer());
}

void GameLogic::draw() {
	stopTimer();
	emit gameEnd(-1);
	this->m_color = -currentPlayer() - 1;
	emit currentPlayerChanged(currentPlayer());
}

void GameLogic::startTimer() {
	this->m_timer.start();
	updateTime();
	this->m_updateTimer.start();
}

void GameLogic::pauseTimer() {
	if (this->m_timer.isActive()) {
		this->m_updateTimer.stop();
		int remain = this->m_timer.remainingTime();
		this->m_timer.stop();
		this->m_time[currentPlayer()] += this->m_timer.interval() - remain;
		this->m_timer.setInterval(remain);
	}
}

void GameLogic::stopTimer() {
	if (this->m_timer.isActive()) {
		this->m_updateTimer.stop();
		int remain = this->m_timer.remainingTime();
		this->m_timer.stop();
		this->m_time[currentPlayer()] += this->m_timer.interval() - remain;
	}
	this->m_timer.setInterval(TURN_TIME * 1000);
}

void GameLogic::updateTime() {
	emit remainTimeChanged(remainTime());
	emit p1TimeChanged(p1Time());
	emit p2TimeChanged(p2Time());
}

int GameLogic::checkWin(int x, int y) {
	if (this->m_steps == N * N) return -1;
	int l, r, u, d, color = this->m_board[x][y];
	for (u = 1; x - u >= 0 && this->m_board[x - u][y] == color; ++u) ;
	for (d = 1; x + d < N && this->m_board[x + d][y] == color; ++d) ;
	for (l = 1; y - l >= 0 && this->m_board[x][y - l] == color; ++l) ;
	for (r = 1; y + r < N && this->m_board[x][y + r] == color; ++r) ;
	if (l + r - 1 >= 5 || u + d - 1 >= 5) return 1;
	int lu, ru, ld, rd;
	for (lu = 1; x - lu >= 0 && y - lu >= 0
	     && this->m_board[x - lu][y - lu] == color; ++lu) ;
	for (ru = 1; x - ru >= 0 && y + ru < N
	     && this->m_board[x - ru][y + ru] == color; ++ru) ;
	for (ld = 1; x + ld < N && y - ld >= 0
	     && this->m_board[x + ld][y - ld] == color; ++ld) ;
	for (rd = 1; x + rd < N && y + rd < N
	     && this->m_board[x + rd][y + rd] == color; ++rd) ;
	if (lu + rd - 1 >= 5 || ld + ru - 1 >= 5) return 1;
	return 0;
}
