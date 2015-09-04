#include "networkmanager.h"

NetworkHostList::NetworkHostList(QObject *parent)
    : QAbstractListModel(parent) {
    
	m_roleNames[NameRole] = "name";
    m_roleNames[AvatarRole] = "avatar";
    m_roleNames[IPRole] = "ip";
    m_roleNames[RoundsRole] = "rounds";
	m_roleNames[WinningRateRole] = "winningRate";
	
	append(NetworkHostData("Somebody", "0.1.2.3", "null", 10, 8, QDateTime::currentDateTime()));
	append(NetworkHostData("Somebody", "0.1.2.3", "null", 10, 8, QDateTime::currentDateTime()));
	append(NetworkHostData("Somebody", "0.1.2.3", "null", 10, 8, QDateTime::currentDateTime()));
	append(NetworkHostData("Somebody", "0.1.2.3", "null", 10, 8, QDateTime::currentDateTime()));
	append(NetworkHostData("Somebody", "0.1.2.3", "null", 10, 8, QDateTime::currentDateTime()));
	append(NetworkHostData("Somebody", "0.1.2.3", "null", 10, 8, QDateTime::currentDateTime()));
	append(NetworkHostData("Somebody", "0.1.2.3", "null", 10, 8, QDateTime::currentDateTime()));
	append(NetworkHostData("Somebody", "0.1.2.3", "null", 10, 8, QDateTime::currentDateTime()));
	append(NetworkHostData("Somebody", "0.1.2.3", "null", 10, 8, QDateTime::currentDateTime()));
	append(NetworkHostData("Huang Dada", "233.233.233.233", "null", 100, 0, QDateTime::currentDateTime()));
	
}

int NetworkHostList::rowCount(const QModelIndex &parent) const {
    Q_UNUSED(parent);
    return m_data.count();
}

QVariant NetworkHostList::data(const QModelIndex &index, int role) const {
    int row = index.row();
    if(row < 0 || row >= m_data.count()) {
        return QVariant();
    }
    const NetworkHostData& host = m_data.at(row);
    switch(role) {
    case NameRole:
        return host.name();
    case AvatarRole:
		return host.avatarId();
	case IPRole:
		return host.ip();
	case RoundsRole:
		return host.rounds();
	case WinningRateRole:
		return host.winningRate();
    }
    return QVariant();
}

QHash<int, QByteArray> NetworkHostList::roleNames() const {
    return m_roleNames;
}

void NetworkHostList::insert(int index, const NetworkHostData &data) {
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

void NetworkHostList::append(const NetworkHostData &data) {
    insert(count(), data);
}

void NetworkHostList::remove(int index) {
    if(index < 0 || index >= m_data.count()) {
        return;
    }
    emit beginRemoveRows(QModelIndex(), index, index);
    m_data.removeAt(index);
    emit endRemoveRows();
    // do not forget to update our count property
    emit countChanged(m_data.count());
}

NetworkManager::NetworkManager(QObject *parent) : QObject(parent) {
	this->m_hostList = new NetworkHostList;
	
	QList<QHostAddress> ipList = QNetworkInterface::allAddresses();
	this->ip = QHostAddress::LocalHost;
	for (QHostAddress addr : ipList) {
		if (addr != QHostAddress::LocalHost && addr.toIPv4Address()) {
			this->ip = addr;
			break;
		}
	}
	
	this->udpSocket.bind(UDP_PORT);
	this->udpSendTimer.setInterval(__udpSendInterval);
	QObject::connect(&this->udpSendTimer, &QTimer::timeout, [this]() {
		if (this->isServer) sendInfo("create");
		else sendInfo("join");
	});
	QObject::connect(&this->udpSocket, &QUdpSocket::readyRead,
	                 this, &NetworkManager::receiveMatchInfo);
	
//	this->udpSendTimer.start();
}

void NetworkManager::sendInfo(const QString &qualifier) {
	QByteArray datagram;
	QDataStream out(&datagram, QIODevice::WriteOnly);
	QDateTime dateTime = QDateTime::currentDateTime();
	out << dateTime;
	out << QString("null");
	out << this->ip.toString();
	out << qualifier;
	udpSocket.writeDatagram(datagram, datagram.size(), QHostAddress::Broadcast, UDP_PORT);
	qDebug() << "send" << dateTime << ip << qualifier;
}

void NetworkManager::startHost() {
	this->setIsHost(true);
	this->pendingOpponent.clear();
	this->udpSendTimer.start();
}

void NetworkManager::abortHost() {
	this->udpSendTimer.stop();
	this->setIsHost(false);
	for (const NetworkHostData &data : this->pendingOpponent) {
		respondJoinRequest(data, false, false);
	}
	this->pendingOpponent.clear();
}

bool NetworkManager::updateHostList(const NetworkHostData &data) {
	bool found = false;
	for (int i = 0; i < this->m_hostList->count(); ++i) {
		NetworkHostData &cur = this->m_hostList->get(i);
		if (cur.dateTime.msecsTo(QDateTime::currentDateTime()) >= __udpTimeoutInterval) {
			this->m_hostList->remove(i);
			--i;
		} else if (data == cur) {
			found = true;
			cur.dateTime = data.dateTime;
		}
	}
	if (!found) {
		this->m_hostList->append(data);
		return true;
	} else {
		return false;
	}
}

void NetworkManager::connectToHost(const QString &ip) {
	assert(this->isHost() == false);
	
}

void NetworkManager::respondJoinRequest(const NetworkHostData &data, bool accepted,
                                        bool triggerNewSignal) {
	assert(this->isHost() == true);
	
	
	if (triggerNewSignal) {
		this->pendingOpponent.pop_front();
		if (this->pendingOpponent.size() > 0) {
			emit newOpponent(this->pendingOpponent.front());
		}
	}
}

void NetworkManager::confirmJoinRequest(const NetworkHostData &data) {
	assert(this->isHost() == false);
}

void NetworkManager::receiveMatchInfo() {
	QByteArray datagram;
	while (udpSocket.hasPendingDatagrams()) {
		datagram.resize(udpSocket.pendingDatagramSize());
		udpSocket.readDatagram(datagram.data(), datagram.size());
		QString ip, profile, qualifier;
		QDateTime dateTime;
		QDataStream in(&datagram, QIODevice::ReadOnly);
		in >> dateTime >> profile >> ip >> qualifier;
		qDebug() << "receive" << dateTime << ip << qualifier;
		if (dateTime.msecsTo(QDateTime::currentDateTime()) >= __udpTimeoutInterval)
			continue;
		NetworkHostData data(profile, ip, "null", 1, 0, dateTime);
		
		if (qualifier == "create") {			// Host: create room
			if (!isHost()) {
				updateHostList(data);
			}
		}
		else if (qualifier == "join") {			// Client: join room request
			assert(this->isHost() == true);
			this->pendingOpponent.append(data);
			if (this->pendingOpponent.size() == 1) {
				emit newOpponent(this->pendingOpponent.front());
			}
		}
		else if (qualifier == "accept") {		// Host: accept join request
			assert(this->isHost() == false);
		}	
		else if (qualifier == "refuse") {		// Host: refuse join request
			assert(this->isHost() == false);
		}
		else if (qualifier == "confirm") {		// Client: confirm join
			assert(this->isHost() == true);
		}
	}
}
