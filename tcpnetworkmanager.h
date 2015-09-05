#ifndef TCPNETWORKMANAGER_H
#define TCPNETWORKMANAGER_H

#include <QObject>
#include <QTcpServer>
#include <QTcpSocket>
#include <QNetworkInterface>
#include <QHostAddress>
#include <QString>
#include <QDataStream>
#include <QByteArray>
#include <QTimer>
#include <QDateTime>
#include <QList>
#include "defineproperty.h"

#include <cassert>

class TcpNetworkManager : public QObject {
	Q_OBJECT
public:
	explicit TcpNetworkManager(QObject *parent = 0);

signals: DEFINE_PROPERTY(int, T, t, ype)
signals: DEFINE_PROPERTY(int, M, m, yself)
signals: DEFINE_PROPERTY(QString, O, o, pponentIP)
signals: DEFINE_PROPERTY(QString, C, c, hatMessage)
	
signals:
	void criticalError(QString message);
	
	void request(QString request);
	void response(QString response);
	void startGame(int player);
	void undo(int player);
	void surrender(int player);
	void exit(int player);
	void draw(int player);
	void showPiece(int x, int y, int player);
	void timeout(int player);
	
public slots:
	Q_INVOKABLE void init(int type, QString ip, QString p1Name, QString p2Name, int first);
	void startTimeout(int msec, QString message);
	void stopTimeout();
	Q_INVOKABLE void gameEnd(int result);
	
	Q_INVOKABLE void receive();
	Q_INVOKABLE void receiveRequest(QString request, int player);
	Q_INVOKABLE void receiveResponse(QString response, int player);
	Q_INVOKABLE void receiveChat(QString message, int player);
	Q_INVOKABLE void receiveReady(int player);
	Q_INVOKABLE void receiveStart(int player);
	Q_INVOKABLE void receiveTimeout(int player);
	Q_INVOKABLE void receiveDropPiece(int x, int y, int player);
	
	Q_INVOKABLE void sendRequest(QString request, int player, QTcpSocket *socket = 0);
	Q_INVOKABLE void sendResponse(QString response, int player, QTcpSocket *socket = 0);
	Q_INVOKABLE void sendChat(QString message, int player, QTcpSocket *socket = 0);
	Q_INVOKABLE void sendReady(int player, QTcpSocket *socket = 0);
	Q_INVOKABLE void sendStart(int player, QTcpSocket *socket = 0);
	Q_INVOKABLE void sendTimeout(int player, QTcpSocket *socket = 0);
	Q_INVOKABLE void sendDropPiece(int x, int y, int player, QTcpSocket *socket = 0);
	
	Q_INVOKABLE void appendChat(const QString &message);
	Q_INVOKABLE void handleError(QAbstractSocket::SocketError socketError);
	
private:
	static const int TCP_PORT = 41013;
	static const int n_qualifier = 11;
	static const QString qualifiers[n_qualifier];
	static const QList<uchar> requests, responses, all;
	
	static uchar getQualifier(const QString &str) {
		for (int i = 0; i < n_qualifier; ++i)
			if (qualifiers[i] == str)
				return (uchar)i;
		return -1;
	}
	static QList<uchar> getQualifier(const QList<QString> &list) {
		QList<uchar> ret;
		for (const QString &str : list)
			ret.append(getQualifier(str));
		return ret;
	}
	
	enum Type {
		Observer = -1,
		Client = 0,
		Host = 1
	};
	/*
	enum Qualifiers {
		Undo = 1 << 0,
		Surrender = 1 << 1,
		Exit = 1 << 2,
		Draw = 1 << 3,
		Accept = 1 << 4,
		Refuse = 1 << 5,
		Chat = 1 << 6,
		Start = 1 << 7,
		Ready = 1 << 8,
		DropPiece = 1 << 9,
		Timeout = 1 << 10,
		Requests = Undo | Surrender | Exit | Draw,
		Responses = Accept | Refuse
		Normal = DropPiece | 
	};*/
	
	QTimer m_timeoutTimer;
	QTcpServer *m_server;
	QTcpSocket *m_chatSocket, *m_socket;
	QList<QTcpSocket *> m_observerList;
	QList<uchar> m_acceptQualifiers;
	QString m_myIP, m_lastRequest, m_names[2];
	bool m_isReady[2];
	quint16 m_blockSize;
	int m_first;
};

#endif // TCPNETWORKMANAGER_H
