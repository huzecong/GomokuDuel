#include "tcpnetworkmanager.h"

const QString TcpNetworkManager::qualifiers[n_qualifier] = {
    "undo", "surrender", "exit", "draw", "load", "save",
    "accept", "refuse",
    "chat", "start", "ready", "droppiece", "timeout", "loaddata"
};
const QList<uchar> TcpNetworkManager::requests = getQualifier
        ({"undo", "surrender", "draw", "exit", "load", "save"});
const QList<uchar> TcpNetworkManager::responses = getQualifier
        ({"accept", "refuse"});
const QList<uchar> TcpNetworkManager::all = getQualifier({
	"undo", "surrender", "exit", "draw", "load", "save",
    "accept", "refuse",
    "chat", "start", "ready", "droppiece", "timeout", "loaddata"
});

TcpNetworkManager::TcpNetworkManager(QObject *parent) : QObject(parent) {
	this->setChatMessage("");
	
	// Find my IP
	this->m_myIP = QHostAddress(QHostAddress::LocalHost).toString();
	QList<QHostAddress> ipAddressesList = QNetworkInterface::allAddresses();
	// use the first non-localhost IPv4 address
	for (int i = 0; i < ipAddressesList.size(); ++i) {
		if (ipAddressesList.at(i) != QHostAddress::LocalHost &&
			ipAddressesList.at(i).toIPv4Address()) {
			this->m_myIP = ipAddressesList.at(i).toString();
			break;
		}
	}
	this->m_blockSize = 0;
}

void TcpNetworkManager::init(int type, QString ip, QString p1Name, QString p2Name, int first) {
	setType(type);
	setOpponentIP(ip);
	this->m_names[0] = p1Name;
	this->m_names[1] = p2Name;
	this->m_first = first;
	
	this->m_blockSize = 0;
	
	qDebug() << opponentIP() << this->m_first << this->myself();
	
	if (type == Host) {
		this->m_server = new QTcpServer(this);
		if (!this->m_server->listen(QHostAddress::Any, TCP_PORT)) {
			emit criticalError("Failed to establish link: " + this->m_server->errorString());
			return ;
		}
		QObject::connect(this->m_server, &QTcpServer::newConnection, [this]() {
			stopTimeout();
			this->m_socket = this->m_server->nextPendingConnection();
			QObject::connect(this->m_socket, &QTcpSocket::disconnected,
			                 this->m_socket, &QTcpSocket::deleteLater);
			QObject::connect(this->m_socket, SIGNAL(error(QAbstractSocket::SocketError)),
			                 this, SLOT(handleError(QAbstractSocket::SocketError)));
			QObject::connect(this->m_socket, &QTcpSocket::readyRead,
			                 this, &TcpNetworkManager::receive);
			QObject::disconnect(this->m_server, &QTcpServer::newConnection, 0, 0);
		});
		startTimeout(2000, "Your opponent may have gone offline\nError: Timer timed out");
	}
	else if (type == Client) {
		this->m_socket = new QTcpSocket(this);
		QObject::connect(this->m_socket, &QTcpSocket::connected,
		                 this, &TcpNetworkManager::stopTimeout);
		QObject::connect(this->m_socket, SIGNAL(error(QAbstractSocket::SocketError)),
		                 this, SLOT(handleError(QAbstractSocket::SocketError)));
		QObject::connect(this->m_socket, &QTcpSocket::readyRead,
		                 this, &TcpNetworkManager::receive);
		QTimer::singleShot(100, [this]() {
			this->m_socket->connectToHost(this->m_opponentIP, TCP_PORT);
			startTimeout(2000, "Your opponent may have gone offline\nError: Timer timed out");
		});
	}
	else if (type == Observer) {
		setType(type);
	}
	else assert(false);
	
//	this->m_acceptQualifiers = getQualifier({"ready", "exit", "chat"});
	this->m_acceptQualifiers = all;
}

void TcpNetworkManager::startTimeout(int msec, QString message) {
	this->m_timeoutTimer.stop();
	this->m_timeoutTimer.setInterval(msec);
	this->m_timeoutTimer.setSingleShot(true);
	QObject::disconnect(&this->m_timeoutTimer, &QTimer::timeout, 0, 0);
	QObject::connect(&this->m_timeoutTimer, &QTimer::timeout, [this, message]() {
		emit criticalError(message);
	});
	this->m_timeoutTimer.start();
}

void TcpNetworkManager::stopTimeout() {
	this->m_timeoutTimer.stop();
	QObject::disconnect(&this->m_timeoutTimer, &QTimer::timeout, 0, 0);
}

void TcpNetworkManager::gameEnd(int result) {
	this->m_isReady[0] = this->m_isReady[1] = false;
	this->m_first ^= 1;
	this->m_lastRequest = "";
	
	if (result == -1) {
		QString message = tr("<u>Game result: <b>Draw</b></u>");
		appendChat(message);
	} else if (result > 0) {
		QString message = tr("<u>Game result: <b>%1</b> won</u>");
		message = message.arg(result == this->m_myself ? "You" : this->m_names[result]);
		appendChat(message);
	}
}

bool TcpNetworkManager::checkFileExists(QString name) {
	QDir dir((QStandardPaths::writableLocation(QStandardPaths::AppDataLocation)));
	QString path = dir.filePath(name);
	return QFile::exists(path);
}

void TcpNetworkManager::receive() {
	QDataStream in(this->m_socket);
	in.setVersion(QDataStream::Qt_4_0);
	
	qDebug() << "receive before check" << this->m_blockSize << this->m_socket->bytesAvailable();
	
	// Ensure message is complete
	if (this->m_blockSize == 0) {
		if (this->m_socket->bytesAvailable() < (int)sizeof(quint16))
			return;
		in >> this->m_blockSize;
		qDebug() << "receive read size" << this->m_blockSize;
	}
	if (this->m_socket->bytesAvailable() < this->m_blockSize)
		return;
	
	// Then read data
	uchar _qualifier;
	int player;
	in >> _qualifier >> player;
	qDebug() << qualifiers[_qualifier] << player;
	if (!this->m_acceptQualifiers.contains(_qualifier)) {
		// Ignore data
		in.skipRawData(this->m_blockSize - 2);
		return ;
	}
	this->m_blockSize = 0;
	
	// Process and broadcast
	QString qualifier = qualifiers[_qualifier];
	if (qualifier == "ready") {
		receiveReady(player);
		if (this->m_type == Host) {
			for (auto socket : this->m_observerList)
				sendReady(player, socket);
		}
	}
	else if (qualifier == "start") {
		receiveStart(player);
	}
	else if (qualifier == "chat") {
		QString message;
		in >> message;
		receiveChat(message, player);
		if (this->m_type == Host) {
			for (auto socket : this->m_observerList)
				sendChat(message, player, socket);
		}
	}
	else if (qualifier == "droppiece") {
		int x, y;
		in >> x >> y;
		receiveDropPiece(x, y, player);
		if (this->m_type == Host) {
			for (auto socket : this->m_observerList)
				sendDropPiece(x, y, player, socket);
		}
	}
	else if (qualifier == "timeout") {
		receiveTimeout(player);
		if (this->m_type == Host) {
			for (auto socket : this->m_observerList)
				sendTimeout(player, socket);
		}
	}
	else if (requests.contains(_qualifier)) {
		if (qualifier == "save" || qualifier == "load") {
			in >> this->m_requestFileName;
		}
		receiveRequest(qualifier, player);
		if (this->m_type == Host) {
			for (auto socket : this->m_observerList)
				sendSaveLoadRequest(qualifier, player, this->m_requestFileName, socket);
		}
	}
	else if (responses.contains(_qualifier)) {
		receiveResponse(qualifier, player);
		if (this->m_type == Host) {
			for (auto socket : this->m_observerList)
				sendResponse(qualifier, player, socket);
		}
	}
	
	if (this->m_socket->bytesAvailable() > 0) {
		QTimer::singleShot(1, this, &TcpNetworkManager::receive);
	}
}

void TcpNetworkManager::receiveRequest(QString request, int player) {
	this->m_lastRequest = request;
	
	QString message = tr("<i>System: <b>%1</b> sent a request for <u>%2</u></i>");
	message = message.arg(player == this->m_myself ? "You" : this->m_names[player]);
	message = message.arg(request);
	appendChat(message);
	
	if (this->m_type != Observer) {
		if (request == "save" || request == "load") {
			if ((request == "load" && !checkFileExists(this->m_requestFileName))
			    || (request == "save" && checkFileExists(this->m_requestFileName))) {
				sendResponse("refuse", this->m_myself);
				for (auto socket : this->m_observerList)
					sendResponse("refuse", this->m_myself, socket);
				return ;
			}
		}
		emit this->request(request);
	}
}

void TcpNetworkManager::receiveResponse(QString response, int player) {
	assert(!this->m_lastRequest.isEmpty());
	if (this->m_type != Observer)
		emit this->response(response);
	
	if (response == "accept") response.append("ed");
	else response.append("d");
	if (this->m_lastRequest == "save" || this->m_lastRequest == "load") {
		QString message = tr("<i>System: <b>%1</b> %2 <b>%3</b> request to <u>%4</u> %5</i>");
		message = message.arg(player == this->m_myself ? "You" : this->m_names[player]);
		message = message.arg(response);
		message = message.arg((player ^ 1) == this->m_myself ? "your" : this->m_names[player ^ 1] + "'s");
		message = message.arg(this->m_lastRequest);
		message = message.arg(this->m_requestFileName);
		appendChat(message);
	} else {
		QString message = tr("<i>System: <b>%1</b> %2 <b>%3</b> <u>%4</u> request</i>");
		message = message.arg(player == this->m_myself ? "You" : this->m_names[player]);
		message = message.arg(response);
		message = message.arg((player ^ 1) == this->m_myself ? "your" : this->m_names[player ^ 1] + "'s");
		message = message.arg(this->m_lastRequest);
		appendChat(message);
	}
	
	if (response == "accepted") {		// Changed due to code above ... =_=
		if (this->m_lastRequest == "undo") {
			emit undo(player ^ 1);
		} else if (this->m_lastRequest == "surrender") {
			emit surrender(player ^ 1);
		} else if (this->m_lastRequest == "exit") {
			emit exit(player ^ 1);
		} else if (this->m_lastRequest == "draw") {
			emit draw(player ^ 1);
		} else if (this->m_lastRequest == "load") {
			assert(this->m_requestFileName != "");
			emit load(player ^ 1, this->m_requestFileName);
		} else if (this->m_lastRequest == "save") {
			assert(this->m_requestFileName != "");
			emit save(player ^ 1, this->m_requestFileName);
		} else assert(false);
	}
	
	this->m_lastRequest = "", this->m_requestFileName = "";
}

void TcpNetworkManager::receiveChat(QString message, int player) {
	if (player == this->m_myself) appendChat("<b>You</b>: " + message);
	else appendChat("<b>" + this->m_names[player] + "</b>: " + message);
}

void TcpNetworkManager::receiveReady(int player) {
	QString message = tr("<i>System: ");
	if (player == this->m_myself) message.append("<b>You</b> are");
	else message.append("<b>" + this->m_names[player] + "</b> is");
	message.append(" ready</i>");
	appendChat(message);
	
	this->m_isReady[player] = true;
	if (this->m_type == Host) {
		if (this->m_isReady[player ^ 1]) {
			sendStart(this->m_first);
			for (auto socket : this->m_observerList)
				sendStart(this->m_first, socket);
			receiveStart(this->m_first);
		}
	}
}

void TcpNetworkManager::receiveStart(int player) {
	this->m_chatMessage.insert(0, "<font color=\"grey\">");
	this->m_chatMessage.append("</font>");
	
	QString message = tr("<i>System: Game started, ");
	if (player == this->m_myself) message.append("<b>you</b> go");
	else message.append("<b>" + this->m_names[player] + "</b> goes");
	message.append(" first</i>");
	appendChat(message);
	
	emit startGame(player);
}

void TcpNetworkManager::receiveTimeout(int player) {
	QString message = tr("<i>System: <b>%1</b> timed out</i>");
	message = message.arg(player == this->m_myself ? "You" : this->m_names[player]);
	appendChat(message);
	
	emit timeout(player);
}

void TcpNetworkManager::receiveDropPiece(int x, int y, int player) {
	emit showPiece(x, y, player);
}

void TcpNetworkManager::sendRequest(QString request, int player, QTcpSocket *socket) {
	QByteArray block;
	QDataStream out(&block, QIODevice::WriteOnly);
	out.setVersion(QDataStream::Qt_4_0);
	
	qDebug() << "send" << "request" << request << player;
	
	// Placeholder for size
	out << (quint16)0;
	// Actual data
	uchar qualifier = getQualifier(request);
	out << qualifier << player;
	// Go back and fill size
	out.device()->seek(0);
	out << (quint16)(block.size() - sizeof(quint16));
	
	if (socket == 0) {
		socket = this->m_socket;
		QString message = tr("<i>System: <b>%1</b> sent a request for <u>%2</u></i>");
		message = message.arg(player == this->m_myself ? "You" : this->m_names[player]);
		message = message.arg(request);
		appendChat(message);
		this->m_lastRequest = request;
	}
	socket->write(block);
}

void TcpNetworkManager::sendSaveLoadRequest(QString request, int player, QString fileName, QTcpSocket *socket) {
	QByteArray block;
	QDataStream out(&block, QIODevice::WriteOnly);
	out.setVersion(QDataStream::Qt_4_0);
	
	qDebug() << "send" << "request" << request << fileName << player;
	
	out << (quint16)0;
	uchar qualifier = getQualifier(request);
	out << qualifier << player << fileName;
	out.device()->seek(0);
	out << (quint16)(block.size() - sizeof(quint16));
	
	if (socket == 0) {
		socket = this->m_socket;
		QString message = tr("<i>System: <b>%1</b> sent a request to <u>%2</u> %3</i>");
		message = message.arg(player == this->m_myself ? "You" : this->m_names[player]);
		message = message.arg(request).arg(fileName);
		appendChat(message);
		this->m_lastRequest = request;
		this->m_requestFileName = fileName;
	}
	socket->write(block);
}

void TcpNetworkManager::sendResponse(QString response, int player, QTcpSocket *socket){
	QByteArray block;
	QDataStream out(&block, QIODevice::WriteOnly);
	out.setVersion(QDataStream::Qt_4_0);
	
	out << (quint16)0;
	uchar qualifier = getQualifier(response);
	out << qualifier << player;
	out.device()->seek(0);
	out << (quint16)(block.size() - sizeof(quint16));
	
	if (socket == 0) {
		socket = this->m_socket;
		
		qDebug() << "send" << "response" << response << player;
		
		if (response == "accept") response.append("ed");
		else response.append("d");
		if (this->m_lastRequest == "save" || this->m_lastRequest == "load") {
			QString message = tr("<i>System: <b>%1</b> %2 <b>%3</b> request to <u>%4</u> %5</i>");
			message = message.arg(player == this->m_myself ? "You" : this->m_names[player]);
			message = message.arg(response);
			message = message.arg((player ^ 1) == this->m_myself ? "your" : this->m_names[player ^ 1] + "'s");
			message = message.arg(this->m_lastRequest);
			message = message.arg(this->m_requestFileName);
			appendChat(message);
		} else {
			QString message = tr("<i>System: <b>%1</b> %2 <b>%3</b> <u>%4</u> request</i>");
			message = message.arg(player == this->m_myself ? "You" : this->m_names[player]);
			message = message.arg(response);
			message = message.arg((player ^ 1) == this->m_myself ? "your" : this->m_names[player ^ 1] + "'s");
			message = message.arg(this->m_lastRequest);
			appendChat(message);
		}
		
		if (response == "accepted") {		// Changed due to code above ... =_=
			qDebug() << "last request" << this->m_lastRequest;
			if (this->m_lastRequest == "undo") {
				emit undo(player ^ 1);
			} else if (this->m_lastRequest == "surrender") {
				emit surrender(player ^ 1);
			} else if (this->m_lastRequest == "exit") {
				emit exit(player ^ 1);
			} else if (this->m_lastRequest == "draw") {
				emit draw(player ^ 1);
			} else if (this->m_lastRequest == "load") {
				assert(this->m_requestFileName != "");
				emit load(player ^ 1, this->m_requestFileName);
			} else if (this->m_lastRequest == "save") {
				assert(this->m_requestFileName != "");
				emit save(player ^ 1, this->m_requestFileName);
			} else assert(false);
		}
		this->m_lastRequest = "", this->m_requestFileName = "";
	}
	socket->write(block);
}

void TcpNetworkManager::sendChat(QString message, int player, QTcpSocket *socket) {
	QByteArray block;
	QDataStream out(&block, QIODevice::WriteOnly);
	out.setVersion(QDataStream::Qt_4_0);
	
	out << (quint16)0;
	uchar qualifier = getQualifier("chat");
	out << qualifier << player;
	out << message;
	out.device()->seek(0);
	out << (quint16)(block.size() - sizeof(quint16));
	
	if (socket == 0) {
		socket = this->m_socket;
		
		qDebug() << "send" << "chat" << message << player;
		
		if (player == this->m_myself) appendChat("<b>You</b>: " + message);
		else appendChat("<b>" + this->m_names[player] + "</b>: " + message);
	}
	socket->write(block);
}

void TcpNetworkManager::sendReady(int player, QTcpSocket *socket) {
	QByteArray block;
	QDataStream out(&block, QIODevice::WriteOnly);
	out.setVersion(QDataStream::Qt_4_0);
	
	out << (quint16)0;
	uchar qualifier = getQualifier("ready");
	out << qualifier << player;
	out.device()->seek(0);
	out << (quint16)(block.size() - sizeof(quint16));
	
	if (socket == 0) {
		socket = this->m_socket;
		
		qDebug() << "send" << "ready" << player;
		
		QString message = tr("<i>System: ");
		if (player == this->m_myself) message.append("<b>You</b> are");
		else message.append("<b>" + this->m_names[player] + "</b> is");
		message.append(" ready</i>");
		appendChat(message);
		
		socket->write(block);
		
		this->m_isReady[player] = true;
		if (this->m_type == Host) {
			if (this->m_isReady[player ^ 1]) {
				sendStart(this->m_first);
				for (auto socket : this->m_observerList)
					sendStart(this->m_first, socket);
				receiveStart(this->m_first);
			}
		}
	} else socket->write(block);
}

void TcpNetworkManager::sendStart(int player, QTcpSocket *socket) {
	QByteArray block;
	QDataStream out(&block, QIODevice::WriteOnly);
	out.setVersion(QDataStream::Qt_4_0);
	
	qDebug() << "send" << "start" << player;
	
	out << (quint16)0;
	uchar qualifier = getQualifier("start");
	out << qualifier << player;
	out.device()->seek(0);
	out << (quint16)(block.size() - sizeof(quint16));
	
	if (socket == 0) socket = this->m_socket;
	socket->write(block);
}

void TcpNetworkManager::sendTimeout(int player, QTcpSocket *socket) {
	QByteArray block;
	QDataStream out(&block, QIODevice::WriteOnly);
	out.setVersion(QDataStream::Qt_4_0);
	
	
	out << (quint16)0;
	uchar qualifier = getQualifier("timeout");
	out << qualifier << player;
	out.device()->seek(0);
	out << (quint16)(block.size() - sizeof(quint16));
	
	if (socket == 0) {
		socket = this->m_socket;
		
		qDebug() << "send" << "timeout" << player;
		
		QString message = tr("<i>System: <b>%1</b> timed out</i>");
		message = message.arg(player == this->m_myself ? "You" : this->m_names[player]);
		appendChat(message);
	}
	socket->write(block);
}

void TcpNetworkManager::sendDropPiece(int x, int y, int player, QTcpSocket *socket) {
	QByteArray block;
	QDataStream out(&block, QIODevice::WriteOnly);
	out.setVersion(QDataStream::Qt_4_0);
	
	out << (quint16)0;
	uchar qualifier = getQualifier("droppiece");
	out << qualifier << player << x << y;
	out.device()->seek(0);
	out << (quint16)(block.size() - sizeof(quint16));
	
	if (socket == 0) {
		socket = this->m_socket;
		
		qDebug() << "send" << "droppiece" << x << y << player;
	}
	socket->write(block);
}

void TcpNetworkManager::appendChat(const QString &message) {
	if (message.isEmpty()) return ;
	this->m_chatMessage.append("<p>" + message + "</p>");
	emit chatMessageChanged(this->m_chatMessage);
}

void TcpNetworkManager::handleError(QAbstractSocket::SocketError socketError) {
	switch (socketError) {
	case QAbstractSocket::RemoteHostClosedError:
		emit criticalError("Your opponent may have gone offline\nError: " + this->m_socket->errorString());
		break;
	case QAbstractSocket::HostNotFoundError:
		emit criticalError("Please check your Internet connection\nError: " + this->m_socket->errorString());
		break;
	case QAbstractSocket::ConnectionRefusedError:
		emit criticalError("Your opponent may have gone offline\nError: " + this->m_socket->errorString());
		break;
	default:
		emit criticalError("Unknown error: " + this->m_socket->errorString());
	}
}
