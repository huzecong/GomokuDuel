#include "udpbroadcastmanager.h"

UdpBroadcastHostList::UdpBroadcastHostList(QObject *parent)
	: QAbstractListModel(parent) {
	
	m_roleNames[NameRole] = "name";
	m_roleNames[AvatarRole] = "avatarId";
	m_roleNames[IPRole] = "ip";
	m_roleNames[UniqueIDRole] = "uniqueId";
}

int UdpBroadcastHostList::rowCount(const QModelIndex &parent) const {
	Q_UNUSED(parent);
	return m_data.count();
}

QVariant UdpBroadcastHostList::data(const QModelIndex &index, int role) const {
	int row = index.row();
	if(row < 0 || row >= m_data.count()) {
		return QVariant();
	}
	const UdpBroadcastHostData& host = m_data.at(row);
	switch(role) {
	case NameRole:
		return host.name;
	case AvatarRole:
		return host.avatarId;
	case IPRole:
		return host.ip;
	case UniqueIDRole:
		return host.uniqueId;
	}
	return QVariant();
}

QHash<int, QByteArray> UdpBroadcastHostList::roleNames() const {
	return m_roleNames;
}

void UdpBroadcastHostList::insert(int index, const UdpBroadcastHostData &data) {
	if(index < 0 || index > m_data.count()) {
		return;
	}
	// view protocol (begin => manipulate => end]
	emit beginInsertRows(QModelIndex(), index, index);
	m_data.insert(index, data);
	emit endInsertRows();
	// update our count property
	emit countChanged(m_data.count());
}

void UdpBroadcastHostList::append(const UdpBroadcastHostData &data) {
	insert(count(), data);
}

void UdpBroadcastHostList::remove(int index) {
	if(index < 0 || index >= m_data.count()) {
		return;
	}
	emit beginRemoveRows(QModelIndex(), index, index);
	m_data.removeAt(index);
	emit endRemoveRows();
	// do not forget to update our count property
	emit countChanged(m_data.count());
}

void UdpBroadcastHostList::clear() {
	emit beginRemoveRows(QModelIndex(), 0, count() - 1);
	m_data.clear();
	emit endRemoveRows();
	// do not forget to update our count property
	emit countChanged(m_data.count());
}

UdpBroadcastManager::UdpBroadcastManager(QObject *parent) : QObject(parent) {
	this->m_hostList = new UdpBroadcastHostList;
	
	refreshIP();
	
	this->udpSocket.bind(UDP_PORT);
	this->udpSendTimer.setInterval(__udpSendInterval);
	QObject::connect(&this->udpSendTimer, &QTimer::timeout, [this]() {
		assert(this->isHost() == true);
		sendInfo("create");
	});
	this->udpRefreshTimer.setInterval(__udpTimeoutInterval / 2);
	QObject::connect(&this->udpRefreshTimer, SIGNAL(timeout()), this, SLOT(updateHostList()));
	QObject::connect(&this->udpSocket, &QUdpSocket::readyRead,
	                 this, &UdpBroadcastManager::receiveMatchInfo);
	
	this->udpRefreshTimer.start();
}

void UdpBroadcastManager::sendInfo(const QString &qualifier, const QHostAddress &ip) {
	QByteArray datagram;
	QDataStream out(&datagram, QIODevice::WriteOnly);
	out.setVersion(QDataStream::Qt_4_0);
	QDateTime dateTime = QDateTime::currentDateTime();
	out << dateTime;
	out << this->m_profileName;
	out << this->m_avatarId;
	out << this->m_uniqueId;
	out << this->ip.toString();
	out << qualifier;
	udpSocket.writeDatagram(datagram, datagram.size(), ip, UDP_PORT);
	qDebug() << "send" << dateTime << this->ip << qualifier;
}

void UdpBroadcastManager::sendInfo(const QString &qualifier, const QString &ip) {
	sendInfo(qualifier, QHostAddress(ip));
}

void UdpBroadcastManager::startHost() {
	this->setIsHost(true);
	this->pendingOpponent.clear();
	this->udpSendTimer.start();
}

void UdpBroadcastManager::abortHost() {
	this->udpSendTimer.stop();
	this->setIsHost(false);
	for (const UdpBroadcastHostData &data : this->pendingOpponent) {
		respondJoinRequest(data.ip, false, false);
	}
	this->pendingOpponent.clear();
}

void UdpBroadcastManager::updateHostList() {
	for (int i = 0; i < this->m_hostList->count(); ++i) {
		UdpBroadcastHostData &cur = this->m_hostList->get(i);
		if (cur.dateTime.msecsTo(QDateTime::currentDateTime()) >= __udpTimeoutInterval) {
			emit hostTimeout(cur.name, cur.ip);
			this->m_hostList->remove(i);
			--i;
		}
	}
}

bool UdpBroadcastManager::updateHostList(const UdpBroadcastHostData &data) {
	bool found = false;
	for (int i = 0; i < this->m_hostList->count(); ++i) {
		UdpBroadcastHostData &cur = this->m_hostList->get(i);
		if (data == cur) {
			found = true;
			cur.dateTime = data.dateTime;
		}
	}
	updateHostList();
	if (!found) {
		this->m_hostList->append(data);
		return true;
	} else {
		return false;
	}
}

void UdpBroadcastManager::connectToHost(QString ip) {
	qDebug() << "connect to host:" << ip;
	assert(this->isHost() == false);
	this->aimHostIP = QHostAddress(ip);
	if (this->aimHostIP.isNull()) {
		emit invalidHostIP(ip);
		assert(false);
	} else {
		sendInfo("join", this->aimHostIP);
	}
}

void UdpBroadcastManager::abortConnectToHost() {
	
}

void UdpBroadcastManager::__newOpponent(const UdpBroadcastHostData &data) {
	emit newOpponent(data.name, data.ip, data.uniqueId, data.avatarId);
}

void UdpBroadcastManager::respondJoinRequest(QString ip, bool accepted,
	                                         bool triggerNewSignal) {
	assert(this->isHost() == true);
	
	if (accepted) sendInfo("accept", ip);
	else sendInfo("refuse", ip);
	
	if (triggerNewSignal) {
		if (this->pendingOpponent.size() == 0) return ;
		this->pendingOpponent.pop_front();
		if (this->pendingOpponent.size() > 0) {
			QTimer::singleShot(1, [this]() {
				this->__newOpponent(this->pendingOpponent.front());
			});
		}
	}
}

void UdpBroadcastManager::confirmJoinRequest(QString ip) {
	assert(this->isHost() == false);
	sendInfo("confirm", ip);
}

void UdpBroadcastManager::reset() {
	if (this->isHost()) {
		abortHost();
	}
	this->udpSendTimer.stop();
	this->udpRefreshTimer.stop();
	this->udpSocket.close();
	this->setIsHost(false);
	this->aimHostIP = this->ip = QHostAddress();
	this->m_hostList->clear();
	this->pendingOpponent.clear();
}

QString UdpBroadcastManager::refreshIP() {
	QList<QHostAddress> ipList = QNetworkInterface::allAddresses();
	this->ip = QHostAddress::LocalHost;
	for (QHostAddress addr : ipList) {
		if (addr != QHostAddress::LocalHost && addr.toIPv4Address()) {
			this->ip = addr;
			return this->ip.toString();
		}
	}
	return "";
}

void UdpBroadcastManager::receiveMatchInfo() {
	QByteArray datagram;
	while (udpSocket.hasPendingDatagrams()) {
		datagram.resize(udpSocket.pendingDatagramSize());
		udpSocket.readDatagram(datagram.data(), datagram.size());
		QString ip, profile, qualifier, avatarId;
		int uniqueId;
		QDateTime dateTime;
		QDataStream in(&datagram, QIODevice::ReadOnly);
		in.setVersion(QDataStream::Qt_4_0);
		in >> dateTime >> profile >> avatarId >> uniqueId >> ip >> qualifier;
		qDebug() << "receive" << dateTime << avatarId << uniqueId << ip << qualifier;
		if (dateTime.msecsTo(QDateTime::currentDateTime()) >= __udpTimeoutInterval)
			continue;
		UdpBroadcastHostData data(profile, ip, avatarId, uniqueId, dateTime);
		
		if (qualifier == "create") {			// Host: create room
			if (this->isHost() == false) {
				updateHostList(data);
			}
		}
		else if (qualifier == "join") {			// Client: join room request
			if (this->isHost() == true) {
				this->pendingOpponent.append(data);
				if (this->pendingOpponent.size() == 1) {
					QTimer::singleShot(1, [this]() {
						this->__newOpponent(this->pendingOpponent.front());
					});
				}
			} else {
				qDebug() << "client received join";
			}
		}
		else if (qualifier == "accept") {		// Host: accept join request
			if (this->isHost() == false) {
				emit requestAccepted(ip);
			} else {
				qDebug() << "host received accept";
			}
		}	
		else if (qualifier == "refuse") {		// Host: refuse join request
			if (this->isHost() == false) {
				emit requestRefused(ip);
			} else {
				qDebug() << "host received refuse";
			}
		}
		else if (qualifier == "confirm") {		// Client: confirm join
			if (this->isHost() == true) {
				emit connectionConfirmed(ip);
			} else {
				qDebug() << "client received confirm";
			}
		} else {
			qDebug() << "unknown signal:" << qualifier;
		}
	}
}
