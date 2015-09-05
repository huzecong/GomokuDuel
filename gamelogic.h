#ifndef GAMELOGIC_H
#define GAMELOGIC_H

#include <QObject>
#include <QString>
#include <QDebug>
#include <QTimer>
#include "defineproperty.h"

#include <cmath>

class GameLogic : public QObject {
	Q_OBJECT
public:
	explicit GameLogic(QObject *parent = 0);
	
	Q_PROPERTY(QString currentColor READ currentColor NOTIFY currentColorChanged)
	QString currentColor() const {
		return this->colors[this->m_color];
	}
	
	Q_PROPERTY(int currentPlayer READ currentPlayer NOTIFY currentPlayerChanged)
	int currentPlayer() const {
		if (this->m_gameStarted == false) return -1;
		return (this->m_first + this->m_color) & 1;
	}
	
	Q_PROPERTY(int p1Score READ p1Score NOTIFY p1ScoreChanged)
	int p1Score() const {
		return this->m_score[0];
	}
	Q_PROPERTY(int p2Score READ p2Score NOTIFY p2ScoreChanged)
	int p2Score() const {
		return this->m_score[1];
	}
	
	Q_PROPERTY(QString p1Time READ p1Time NOTIFY p1TimeChanged)
	QString p1Time() const {
		int second = this->m_time[0];
		if (currentPlayer() == 0)
			second += this->m_timer.interval() - this->m_timer.remainingTime();
		second /= 1000;
		int minute = second / 60;
		second %= 60;
		QString res = "%1%2:%3%4";
		if (minute >= 10) res = res.arg("").arg(minute);
		else res = res.arg("0").arg(minute);
		if (second >= 10) res = res.arg("").arg(second);
		else res = res.arg("0").arg(second);
		return res;
	}
	Q_PROPERTY(QString p2Time READ p2Time NOTIFY p2TimeChanged)
	QString p2Time() const {
		int second = this->m_time[1];
		if (currentPlayer() == 1)
			second += this->m_timer.interval() - this->m_timer.remainingTime();
		second /= 1000;
		int minute = second / 60;
		second %= 60;
		QString res = "%1%2:%3%4";
		if (minute >= 10) res = res.arg("").arg(minute);
		else res = res.arg("0").arg(minute);
		if (second >= 10) res = res.arg("").arg(second);
		else res = res.arg("0").arg(second);
		return res;
	}
	Q_PROPERTY(bool canUndo READ canUndo NOTIFY canUndoChanged)
	bool canUndo() const;
	Q_PROPERTY(int turnTime READ turnTime)
	int turnTime() const {
		return TURN_TIME;
	}
	Q_PROPERTY(int remainTime READ remainTime NOTIFY remainTimeChanged)
	int remainTime() const {
		return ceil((double)this->m_timer.remainingTime() / 1000);
	}
	
signals: DEFINE_PROPERTY(int, M, m, yself)
signals: DEFINE_PROPERTY(bool, G, g, ameStarted)
	
signals:
	void currentColorChanged(QString currentColor);
	void currentPlayerChanged(int currentPlayer);
	void p1ScoreChanged(int score);
	void p2ScoreChanged(int score);
	void p1TimeChanged(QString time);
	void p2TimeChanged(QString time);
	void canUndoChanged(bool canUndo);
	void remainTimeChanged(int remainTime);
	
	void resetAll();
	void showPiece(int x, int y, QString color);
	void removePiece(int x, int y);
	void gameEnd(int result);
	void timeout(int player);
	
public slots:
	Q_INVOKABLE void init(int first = 0);
	Q_INVOKABLE void newGame();
	Q_INVOKABLE bool canDrop(int x, int y) const;
	Q_INVOKABLE void dropPiece(int x, int y);
	Q_INVOKABLE void nextTurn();
	
	Q_INVOKABLE void undo(int player);
	Q_INVOKABLE void surrender(int player);
	Q_INVOKABLE void draw();
	
	Q_INVOKABLE void startTimer();
	Q_INVOKABLE void pauseTimer();
	Q_INVOKABLE void stopTimer();
	void updateTime();
	
private:
	enum {
		None = -1,
		Black = 0,
		White = 1
	};
	static const int TURN_TIME = 20;
	
	inline int index(int x, int y) const {
		return x * N + y;
	}
	inline int colorAt(int id) const {
		int x = id / N, y = id % N;
		return this->m_board[x][y];
	}
	inline int colorToPlayer(int color) const {
		if (color == Black) return this->m_first;
		else return this->m_first ^ 1;
	}
	inline int playerToColor(int player) const {
		if (player == this->m_first) return Black;
		else return White;
	}

	int checkWin(int x, int y);
	
	static const int N = 15;
	static const QString colors[2];
	int m_score[2], m_color, m_board[N][N], m_first;
	int m_history[N * N], m_steps, m_time[2];
	QTimer m_timer, m_updateTimer;
};

#endif // GAMELOGIC_H
