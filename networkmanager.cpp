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
        return host.name;
    case AvatarRole:
		return host.avatarId;
	case IPRole:
		return host.ip;
	case RoundsRole:
		return host.rounds;
	case WinningRateRole:
		return (double)host.win * 100.0 / host.rounds;
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
	
	this->udpSendTimer.start();
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

void NetworkManager::receiveMatchInfo() {
	QByteArray datagram;
	while (udpSocket.hasPendingDatagrams()) {
		datagram.resize(udpSocket.pendingDatagramSize());
		udpSocket.readDatagram(datagram.data(), datagram.size());
		QString ip, profile, qualifier;
		QDateTime dateTime;
		QDataStream in(&datagram, QIODevice::ReadOnly);
		in >> dateTime >> profile >> ip >> qualifier;
		if (QHostAddress(ip) == this->ip) continue ;
		qDebug() << "receive" << dateTime << ip << qualifier;
		if (dateTime.msecsTo(QDateTime::currentDateTime()) >= __udpTimeoutInterval)
			continue;
		NetworkHostData data(profile, ip, "null", 1, 0, dateTime);
		
		bool found = false;
		for (int i = 0; i < this->m_hostList->count(); ++i) {
			const NetworkHostData &cur = this->m_hostList->get(i);
			if (cur.dateTime.msecsTo(QDateTime::currentDateTime()) >= __udpTimeoutInterval) {
				this->m_hostList->remove(i);
				--i;
			} else if (data == cur) {
				found = true;
			}
		}
		if (!found) this->m_hostList->append(data);
	}
}
